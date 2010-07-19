#ifndef MPHF_HASH_IMPL_HH
#define MPHF_HASH_IMPL_HH

#include <cstdlib>
#include <cstring>

namespace MPHF {
  class HashImpl {
  public:
    struct Param {
      Param(unsigned ini, unsigned mul, unsigned eor) 
	: ini(ini), mul(mul), eor(eor) {}
      Param() 
	: ini(rand()%0x1000+1), mul(rand()%0x1000+3), eor(rand()%0x1000+2) {}
      
      const unsigned ini;
      const unsigned mul;
      const unsigned eor;
    };
    
    HashImpl(unsigned ini, unsigned mul, unsigned eor) : param(ini,mul,eor) {}
    HashImpl() {}
    
    static void set_seed(unsigned seed) { srand(seed); }
    
    unsigned hash(const char* key, unsigned size) const {
      const unsigned* uk = reinterpret_cast<const unsigned*>(key);
      
      unsigned h = param.ini;
      unsigned i = 0;
      
      for(; i < size; i += sizeof(unsigned), uk++) 
	h = (h*param.mul + *uk) ^ param.eor;
      
      for(; i < size; i++) 
	h = (h*param.mul + key[i]) ^ param.eor;
      
      return h;
    }
    
    unsigned hash(const char* key) const {
      return hash(key, strlen(key));
    }
    
    const Param param;
  };
}
  
#endif
