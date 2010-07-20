#ifndef MPHF_BIT_VECTOR_HH
#define MPHF_BIT_VECTOR_HH

#include <cstring>

namespace MPHF {
  class BitVector {
    const static unsigned PER_BLOCK_SIZE = sizeof(unsigned)*8;
    const static unsigned RANK_INDEX_INTERVAL = PER_BLOCK_SIZE*8;
    const static unsigned BR_RATE = RANK_INDEX_INTERVAL/PER_BLOCK_SIZE;

  public:
    static BitVector* allocate(unsigned bit_length, bool need_rank_index=false) {
      unsigned* blocks = new unsigned[ceil(bit_length,PER_BLOCK_SIZE)];
      memset(blocks, 0, sizeof(unsigned)*ceil(bit_length, PER_BLOCK_SIZE));
      
      return 
	new BitVector(bit_length, blocks,
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
	  for(unsigned j=0; j < BR_RATE; j++)
	    acc_1bit_count += log_count(blocks[i*BR_RATE+j]);
      }
    }
    
    unsigned rank(unsigned pos) const {
      unsigned ri     = pos/RANK_INDEX_INTERVAL;
      unsigned offset = pos%RANK_INDEX_INTERVAL;
      return rank_index[ri] + block_rank(ri*BR_RATE, offset) - 1;
    }

    unsigned block_size() const { return ceil(bit_length, PER_BLOCK_SIZE); }
    unsigned rank_index_size() const { return ceil(bit_length, RANK_INDEX_INTERVAL); }
    
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
      unsigned i=0;
      for(; offset >= 32; i++, offset-=32)
	r += log_count(blocks[block_num+i]);
      return r + log_count(blocks[block_num+i] & ((2 << offset)-1));
    }
    
  public:
    const unsigned  bit_length;
    const unsigned* blocks;
    const unsigned* rank_index;
  };
}

#endif
