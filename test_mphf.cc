#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "hash.hh"

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cerr << "Usage: test_mphf <index>" << std::endl;
    return 1;
  }

  const char* index_filepath = argv[1];
  
  MPHF::Hash hash(index_filepath);
  if(!hash) {
    std::cerr << "Can't open file: " << index_filepath << std::endl;
    return 1;
  }
    
  std::string line;
  std::vector<unsigned> hash_values;

  std::cerr << "= Read keyset and calculate hash values:" << std::endl;
  while(std::getline(std::cin, line)) 
    hash_values.push_back(hash.mphf(line.c_str()));
  std::cerr << "  == key count: " << hash_values.size() << std::endl;
  std::cerr << "DONE" << std::endl;
  
  std::cerr << "= Uniqueness check: " << std::endl;
  std::sort(hash_values.begin(), hash_values.end());
  std::vector<unsigned>::const_iterator end = std::unique(hash_values.begin(), hash_values.end());
  if(end == hash_values.end()) {
    std::cerr << "  == OK" << std::endl;
  } else {
    unsigned dup_rate = 100 - (((end-hash_values.begin())*100)/hash_values.size());
    std::cerr << "  == failure: duplicate rate is " << dup_rate << "%" << std::endl;
  }
  std::cerr << "DONE" << std::endl;

  return 0;
}
