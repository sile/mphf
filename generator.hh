#ifndef MPHF_GENERATOR_HH
#define MPHF_GENERATOR_HH

#include "hash_impl.hh"
#include "bit_vector.hh"
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

namespace MPHF {
  class Generator {
    struct Edge {
      unsigned vertices[2];
      unsigned& operator[](unsigned index) { return vertices[index]; }
      unsigned operator[](unsigned index) const { return vertices[index]; }
    };
    
    struct VertexLessThan {
      VertexLessThan(unsigned nth_vertex) : nth_vertex(nth_vertex) {}
      
      bool operator()(const Edge& e1, const Edge& e2) const {
	return e1[nth_vertex] < e2[nth_vertex];
      }
      
      unsigned nth_vertex;
    };
      
  public:
    Generator(const char* keyset_filepath, double c=2.09, int hash_seed=-1) 
      : keyset_filepath(keyset_filepath), h(NULL), graph(NULL), 
	rank_bv(NULL), hash_assign_bv(NULL),
	key_count(count_line(keyset_filepath)), hash_value_limit(key_count*c) {
      HashImpl::set_seed(static_cast<unsigned>(hash_seed != -1 ? hash_seed : time(NULL)));
      
      graph = new Edge[key_count+1];
    }
    
    ~Generator() {
      delete [] h;
      delete [] graph;
      BitVector::free(rank_bv);
      BitVector::free(hash_assign_bv);
    }

    operator bool() const { return key_count != 0; }

    bool mapping() {
      remake_hash_impl();
      generate_graph();
      return check_and_sort_graph();
    }

    void assigning() {
      std::vector<bool> visited(hash_value_limit, false);
      std::vector<char> assign_tmp(hash_value_limit, 2);

      for(unsigned i=0; i < key_count; i++) {
	for(unsigned nth=0; nth < 2; nth++) {
	  unsigned v = graph[i][nth];
	  if(visited[v] == false) {
	    char tmp = nth;
	    for(unsigned nth=0; nth < 2; nth++)
	      tmp -= assign_tmp[graph[i][nth]];
	    assign_tmp[v] = abs(tmp%2);
	    break;
	  }
	}

	for(unsigned nth=0; nth < 2; nth++)
	  visited[graph[i][nth]] = true;
      }
      delete [] graph;
      graph = NULL;

      rank_bv        = BitVector::allocate(hash_value_limit, true);
      hash_assign_bv = BitVector::allocate(key_count, false);

      unsigned use_value_count=0;
      for(unsigned i=0; i < hash_value_limit; i++) {
	if(assign_tmp[i] != 2) {
	  if(assign_tmp[i] == 1)
	    hash_assign_bv->set_1bit(use_value_count);
	  use_value_count++;
	  
	  rank_bv->set_1bit(i);
	}
      }
    }
    
    void ranking() {
      rank_bv->build_rank_index();
    }
    
    void save(std::ofstream& out) const {
      out.write(reinterpret_cast<const char*>(&key_count), sizeof(unsigned));
      out.write(reinterpret_cast<const char*>(&hash_value_limit), sizeof(unsigned));
      
      out.write(reinterpret_cast<const char*>(&h[0].param), sizeof(HashImpl::Param));
      out.write(reinterpret_cast<const char*>(&h[1].param), sizeof(HashImpl::Param));

      out.write(reinterpret_cast<const char*>(&rank_bv->bit_length), sizeof(unsigned));      
      out.write(reinterpret_cast<const char*>(rank_bv->blocks), sizeof(unsigned)*rank_bv->block_size()); 
      out.write(reinterpret_cast<const char*>(rank_bv->rank_index), sizeof(unsigned)*rank_bv->rank_index_size()); 

      out.write(reinterpret_cast<const char*>(&hash_assign_bv->bit_length), sizeof(unsigned));      
      out.write(reinterpret_cast<const char*>(hash_assign_bv->blocks), sizeof(unsigned)*hash_assign_bv->block_size()); 
    }

  private:
    void remake_hash_impl () {
      delete [] h;
      h = new HashImpl[2];
    }

    unsigned count_line(const char* filepath) const {
      unsigned count=0;
      std::ifstream in(filepath);
      if(!in)
	return 0;
      
      std::string line;
      while(std::getline(in, line))
	count++;
      
      return count;
    }    

    void generate_graph() {
      const unsigned middle = hash_value_limit/2;
      std::ifstream in(keyset_filepath);
      std::string key;
      for(unsigned i=0; std::getline(in,key); i++) {
	graph[i][0] = h[0].hash(key.c_str()) % middle;
	graph[i][1] = h[1].hash(key.c_str()) % middle + middle;
      }
      
      // sentinel value
      graph[key_count][0] = hash_value_limit+1;
      graph[key_count][1] = hash_value_limit+1;
    }

    // Is the graph acyclic?
    bool check_and_sort_graph() {
      unsigned beg=0;
      unsigned prev=0;
      unsigned side=0;
      for(; beg < key_count; side=(side+1)%2) {
	VertexLessThan vtl(side);
	std::sort(graph+beg, graph+key_count, vtl);
	
	for(unsigned i=beg; i < key_count; i++)
	  if(graph[i][side] != graph[i+1][side])
	    std::swap(graph[beg++], graph[i]);   // leaf vertex
	  else
	    for(i++; graph[i][side] == graph[i+1][side]; i++);
	
	if(prev==beg)
	  return false;
	prev=beg;
      }
      std::reverse(graph, graph+key_count);
      return true;
    }
    
  private:
    const char* keyset_filepath;
    HashImpl* h;
    Edge* graph;
    BitVector* rank_bv;
    BitVector* hash_assign_bv;
    
  public:
    const unsigned key_count;
    const unsigned hash_value_limit;
  };
}

#endif
