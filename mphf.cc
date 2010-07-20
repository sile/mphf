#include <iostream>
#include <string>
#include "hash.hh"

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cerr << "Usage: mphf <index>" << std::endl;
    return 1;
  }

  const char* index_filepath = argv[1];
  
  MPHF::Hash hash(index_filepath);
  if(!hash) {
    std::cerr << "Can't open file: " << index_filepath << std::endl;
    return 1;
  }

  std::string line;

  while(std::getline(std::cin, line)) 
    std::cout << hash.mphf(line.c_str()) << std::endl;

  return 0;
}
