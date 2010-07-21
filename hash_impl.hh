#ifndef MPHF_HASH_IMPL_HH
#define MPHF_HASH_IMPL_HH

#include <cstdlib>
#include <cstring>

namespace MPHF {
  class HashImpl {
  public:
    struct Param {
      Param(unsigned ini, unsigned mul)
	: ini(ini), mul(mul) {} 
      Param() 
	: ini(rand()), mul(rand()%224+32) {} 
      
      const unsigned ini;
      const unsigned mul;
    };
    
    HashImpl(unsigned ini, unsigned mul) : param(ini,mul) {}
    HashImpl() {}
    
    static void set_seed(unsigned seed) { srand(seed); }
    
    static const unsigned ui_size_minus8 = sizeof(unsigned)*8 - 8;

    unsigned hash(const char* key, unsigned size) const {
      const unsigned* u = (const unsigned*)(key);
      unsigned h = param.ini;
      unsigned i = sizeof(unsigned);
      for(; i <= size; i += sizeof(unsigned), u++) 
	h = h*param.mul ^ *u + (h>>ui_size_minus8);
      
      if((i-=sizeof(unsigned)) != size)
	for(; i < size; i++)
	  h = h*param.mul ^ key[i] + (h>>ui_size_minus8);
      
      return h;
    }
    
    unsigned hash(const char* key) const {
      return hash(key, strlen(key));
    }
    
    const Param param;
  };
}
  
#endif
