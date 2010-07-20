#include <iostream>
#include <fstream>
#include <string>
#include "generator.hh"

int count_line(const char* filepath) {
  int count=0;
  std::ifstream in(filepath);
  if(!in)
    return -1;
  
  std::string line;
  while(getline(in, line))
    count++;

  return count;
}

int main(int argc, char** argv) {
  if(argc != 3) {
    std::cerr << "Usage: mkmphf [--mapping-try-limit=N] [--c=D] <index> <keyset>" << std::endl;
    return 1;
  }
  
  const char* keyset_filepath = argv[2];
  const char* index_filepath = argv[1];
  
  std::cerr << "= Initialize" << std::endl;
  MPHF::Generator gen(keyset_filepath);
  std::cerr << "  == key count: " << gen.key_count << std::endl;
  std::cerr << "  == hash value limit: " << gen.hash_value_limit << std::endl;
  std::cerr << "DONE" << std::endl;
  
  std::cerr << "= Mapping Step: " << std::endl;
  int try_count=0;
  for(; try_count < 32; try_count++) {
    std::cerr << " == Try: " << try_count+1 << '/' << 32 << std::endl;
    if(gen.mapping()==true)
      break;
  }
  if(try_count==32) {
    std::cerr << "Failure: try limit exceeded" << std::endl;
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
