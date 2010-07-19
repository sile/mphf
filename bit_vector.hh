#ifndef MPHF_BIT_VECTOR_HH
#define MPHF_BIT_VECTOR_HH

namespace MPHF {
  class BitVector {
    const static unsigned PER_BLOCK_SIZE = sizeof(unsigned)*8;
    const static unsigned RANK_INDEX_INTERVAL = PER_BLOCK_SIZE*3;

  public:
    static BitVector* allocate(unsigned bit_length, bool need_rank_index=false) {
      return 
	new BitVector(bit_length, 
		      new unsigned[ceil(bit_length,PER_BLOCK_SIZE)],
		      need_rank_index ? new unsigned[ceil(bit_length,RANK_INDEX_INTERVAL)] : NULL);
    }
    static void free(BitVector* bv) {
      delete [] bv->blocks;
      delete [] bv->rank_index;
    }

    BitVector(unsigned bit_length, const unsigned* blocks, const unsigned* rank_index) 
      : bit_length(bit_length), blocks(blocks), rank_index(rank_index) {}

    void set_1bit(unsigned pos) {
      unsigned* b = const_cast<unsigned*>(blocks);
      b[pos/PER_BLOCK_SIZE] |= 1<<(pos%PER_BLOCK_SIZE);
    }
    
    bool is_1bit(unsigned pos) const {
      return blocks[pos/PER_BLOCK_SIZE] & (1<<(pos%PER_BLOCK_SIZE));
    }

    void build_rank_index() {
      unsigned* ri = const_cast<unsigned*>(rank_index);
      unsigned limit = ceil(bit_length, RANK_INDEX_INTERVAL);
      unsigned acc_1bit_count=0;
      for(unsigned i=0; i < limit-1; i++) {
	ri[i] = acc_1bit_count;
	acc_1bit_count += log_count(blocks[i*2])+log_count(blocks[i*2+1]);
      }
      if(limit != 0)
	ri[limit-1] = acc_1bit_count;
    }
    
    unsigned rank(unsigned pos) const {
      unsigned ri     = pos/RANK_INDEX_INTERVAL;
      unsigned offset = pos%RANK_INDEX_INTERVAL;
      return rank_index[ri] + block_rank(ri*2, offset);
    }

    unsigned block_size() const { return ceil(bit_length, PER_BLOCK_SIZE); }
    unsigned rank_index_size() const { return ceil(bit_length, RANK_INDEX_INTERVAL); }
    
  private:
    static unsigned log_count(unsigned n) {
      n = ((n & 0xAAAAAAAA) >> 1) + (n & 0x55555555);
      n = ((n & 0xCCCCCCCC) >> 2) + (n & 0x33333333);
      n = ((n >> 4) + n) & 0x0F0F0F0F;
      n = (n>>8) + n;
      return ((n>>16) + n) & 0x3F;
    }

    static unsigned ceil(unsigned dividend, unsigned divisor) {
      return (dividend + (divisor - 1)) / divisor;
    }

    unsigned block_rank(unsigned block_num, unsigned offset) const {
      if(offset < 32)
	return log_count(blocks[block_num] & ((2 << offset)-1));

      return log_count(blocks[block_num]) +
	     log_count(blocks[block_num+1] & ((2 << (offset-32))-1));
    }
    
  public:
    const unsigned  bit_length;
    const unsigned* blocks;
    const unsigned* rank_index;
  };
}

#endif
