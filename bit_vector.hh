#ifndef MPHF_BIT_VECTOR_HH
#define MPHF_BIT_VECTOR_HH

#include <cstring>

namespace MPHF {
  class BitVector {
    const static unsigned PER_BLOCK_SIZE = sizeof(unsigned)*8;
    const static unsigned RANK_INDEX_INTERVAL = PER_BLOCK_SIZE*8;
    const static unsigned BR_TIMES = RANK_INDEX_INTERVAL/PER_BLOCK_SIZE;

  public:
    static BitVector* allocate(unsigned bit_length, bool use_rank=false) {
      unsigned* blocks = new unsigned[block_size(bit_length)];
      memset(blocks, 0, sizeof(unsigned)*block_size(bit_length));
      
      return new BitVector(bit_length, blocks, use_rank ? new unsigned[rank_index_size(bit_length)] : NULL);
    }
    
    static void free(BitVector* bv) {
      if(bv) {
	delete [] bv->blocks;
	delete [] bv->rank_index;
      }
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

    unsigned get(unsigned pos) const {
      return static_cast<unsigned>(is_1bit(pos));
    }

    void build_rank_index() {
      unsigned* ri = const_cast<unsigned*>(rank_index);
      unsigned limit = rank_index_size();
      unsigned acc_1bit_count=0;
      for(unsigned i=0; i < limit; i++) {
	ri[i] = acc_1bit_count;
	if(i != limit-1)
	  for(unsigned j=0; j < BR_TIMES; j++)
	    acc_1bit_count += log_count(blocks[i*BR_TIMES+j]);
      }
    }
    
    unsigned rank(unsigned pos) const {
      unsigned ri     = pos/RANK_INDEX_INTERVAL;
      unsigned offset = pos%RANK_INDEX_INTERVAL;
      return rank_index[ri] + block_rank(ri*BR_TIMES, offset) - 1;
    }

    unsigned block_size() const { return block_size(bit_length); }
    unsigned rank_index_size() const { return rank_index_size(bit_length); }
    
    static unsigned block_size(unsigned bit_length) { return ceil(bit_length, PER_BLOCK_SIZE); }
    static unsigned rank_index_size(unsigned bit_length) { return ceil(bit_length, RANK_INDEX_INTERVAL); }
    
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
      unsigned r=0;
      for(; offset >= 32; block_num++, offset-=32)
	r += log_count(blocks[block_num]);
      return r + log_count(blocks[block_num] & ((2 << offset)-1));
    }
    
  public:
    const unsigned  bit_length;
    const unsigned* const blocks;
    const unsigned* const rank_index;
  };
}

#endif
