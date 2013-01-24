CXX=g++

CXXFLAGS=-I/darwin/Linux/include/ -I/darwin/Framework/include/ -I/usr/include/opencv2 -I/usr/include/eigen3 -g -std=c++0x -O3 -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

LDFLAGS=-lpthread -lrt `pkg-config opencv --libs`

TARGET=main

SOURCES=main.cc Agent/run.cc Agent/init.cc Agent/think.cc

HEADERS=Agent/agent.hh Agent/agent.ih

OBJECTS=${SOURCES:.cc=.o}

all: ${TARGET} darwin.a

clean:
	rm ${OBJECTS}

darwin.a:
	make -C /darwin/Linux/build

${OBJECTS}: ${HEADERS}

${TARGET}: darwin.a ${OBJECTS} ${HEADERS}
	${CXX} ${OBJECTS} /darwin/Linux/lib/darwin.a ${LDFLAGS} -o ${TARGET}