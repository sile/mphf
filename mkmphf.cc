#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "generator.hh"

#define OPT1 "--mapping-loop-limit"
#define OPT2 "--hash-code-scale"

int main(int argc, char** argv) {
  if(argc < 3) {
    std::cerr << "Usage: mkmphf [--mapping-loop-limit=<N>] [--hash-code-scale=<D>] <index> <keyset>" << std::endl;
    return 1;
  }

  int param_i=1;
  int loop_limit=32;
  double hash_code_scale=2.09;
  
  for(; param_i < argc; param_i++) {
    if(strncmp(argv[param_i], OPT1"=", strlen(OPT1"="))==0) {
      loop_limit = atoi(argv[param_i]+strlen(OPT1"="));
    } else if (strncmp(argv[param_i], OPT2"=", strlen(OPT2"="))==0) {
      hash_code_scale = atof(argv[param_i]+strlen(OPT2"="));
    } else if (strncmp(argv[param_i], "--", 2) != 0) {
      break;
    } else {
      std::cerr << "Usage: mkmphf [--mapping-loop-limit=<N>] [--hash-code-scale=<D>] <index> <keyset>" << std::endl;
      return 1;      
    }
  }

  const char* keyset_filepath = argv[param_i+1];
  const char* index_filepath = argv[param_i];

  std::cerr << "= Initialize" << std::endl;
  MPHF::Generator gen(keyset_filepath, hash_code_scale);
  if(!gen) {
    std::cerr << "Can't open file(or file is empty): " << keyset_filepath << std::endl;
    return 1;    
  }

  std::cerr << "  == key count: " << gen.key_count << std::endl;
  std::cerr << "  == hash value limit: " << gen.hash_value_limit << std::endl;
  std::cerr << "DONE" << std::endl;
  
  std::cerr << "= Mapping Step: " << std::endl;
  int loop_count=0;
  for(; loop_count < loop_limit; loop_count++) {
    std::cerr << " == loop: " << loop_count+1 << '/' << loop_limit << std::endl;
    if(gen.mapping()==true)
      break;
  }
  if(loop_count==loop_limit) {
    std::cerr << "Failure: loop limit exceeded" << std::endl;
    return 3;
  }
  std::cerr << "Done" << std::endl;
    
  std::cerr << "= Assigning Step: " << std::endl;
  gen.assigning();
  std::cerr << "Done" << std::endl;

  std::cerr << "= Ranking Step: " << std::endl;
  gen.ranking();
  std::cerr << "Done" << std::endl;

  std::cerr << "= Save: " << index_filepath << std::endl;
  std::ofstream out(index_filepath);
  if(!out) {
    std::cerr << "Can't open file: " << index_filepath << std::endl;
    return 4;
  }
  gen.save(out);
  std::cerr << "Done" << std::endl;
  return 0;
}
