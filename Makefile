CXXFLAGS=-O2 -Wall
CXX=g++

all: mkmphf test_mphf mphf

mkmphf: mkmphf.cc generator.hh hash_impl.hh bit_vector.hh
	${CXX} ${CXXFLAGS} -o ${@} ${@}.cc

test_mphf: test_mphf.cc hash.hh bit_vector.hh
	${CXX} ${CXXFLAGS} -o ${@} ${@}.cc

mphf: mphf.cc hash.hh bit_vector.hh
	${CXX} ${CXXFLAGS} -o ${@} ${@}.cc

clean: 
	rm -f mkmphf test_mphf mphf
