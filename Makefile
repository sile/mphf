CXXFLAGS= -O2 -Wall
CXX=g++

all: mkmphf test_mphf mphf

mkmphf: mkmphf.cc generator.hh hash_impl.hh bit_vector.hh
	${CXX} ${CXXFLAGS} -o ${@} mkmphf.cc

test_mphf: test_mphf.cc hash.hh
	${CXX} ${CXXFLAGS} -o ${@} test_mphf.cc

mphf: mphf.cc hash.hh 
	${CXX} ${CXXFLAGS} -o ${@} mphf.cc

clean: 
	rm -f mkmphf test_mphf mphf
