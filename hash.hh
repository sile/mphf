#ifndef MPHF_HASH_HH
#define MPHF_HASH_HH

#include "bit_vector.hh"
#include "hash_impl.hh"
#include "mmap_t.hh"
#include <cstring>

namespace MPHF {
  class Hash {
    struct Data {
      Data(const char* filepath) 
	: mm(filepath), key_count(0), hash_value_limit(0), middle(0),
	  h(NULL), rank_bv(NULL), hash_assign_bv(NULL) {
	if(!mm)
	  return;
	
	const char* p=static_cast<const char*>(mm.ptr);
	
	key_count        = reinterpret_cast<const unsigned*>(p)[0];
	hash_value_limit = reinterpret_cast<const unsigned*>(p)[1];
	middle = hash_value_limit/2;
	p += sizeof(unsigned)*2;
	
	h = reinterpret_cast<const HashImpl*>(p);
        p += sizeof(HashImpl)*2;
      
        unsigned bit_length = reinterpret_cast<const unsigned*>(p)[0];
        p += sizeof(unsigned);
	rank_bv = new BitVector(bit_length, 
				reinterpret_cast<const unsigned*>(p), 
				reinterpret_cast<const unsigned*>(p)+BitVector::block_size(bit_length));
        p += sizeof(unsigned)*(BitVector::block_size(bit_length)+BitVector::rank_index_size(bit_length));

	bit_length = reinterpret_cast<const unsigned*>(p)[0];
	p += sizeof(unsigned);
	hash_assign_bv = new BitVector(bit_length, reinterpret_cast<const unsigned*>(p), NULL);
      }

      ~Data() {
	delete rank_bv;
	delete hash_assign_bv;
      }
      
      unsigned h1(const char* key, unsigned size) const {
	return h[0].hash(key,size) % middle;
      }

      unsigned h2(const char* key, unsigned size) const {
	return h[1].hash(key,size) % middle + middle;
      }

      mmap_t mm;
      unsigned key_count;
      unsigned hash_value_limit;
      unsigned middle;
      const HashImpl* h;
      const BitVector* rank_bv;
      const BitVector* hash_assign_bv;
    };

  public:
    Hash(const char* filepath) 
      : d(filepath) {}
    
    operator bool () const { return d.mm; }
    
    unsigned mphf(const char* key, unsigned size) const {
      unsigned h1 = d.h1(key,size);
      unsigned h2 = d.h2(key,size);
      
      if(d.rank_bv->is_1bit(h1)==false)
	return d.rank_bv->rank(h2);
      if(d.rank_bv->is_1bit(h2)==false)
	return d.rank_bv->rank(h1);

      unsigned v[2] = {d.rank_bv->rank(h1), d.rank_bv->rank(h2)};
      unsigned i = d.hash_assign_bv->get(v[0])+d.hash_assign_bv->get(v[1]);
      return v[i%2];
    }
    
    unsigned mphf(const char* key) const {
      return mphf(key, strlen(key));
    }
    
  private:
    const Data d;
  };
}

#endif
