CXXFLAGS= -O2 -Wall
CXX=g++

mkmphf: mkmphf.cc generator.hh hash_impl.hh bit_vector.hh
	${CXX} ${CXXFLAGS} -o ${@} mkmphf.cc
