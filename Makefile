CXX=g++


DARWINDIR=robotis
VISIONDIR=../vision

CXXFLAGS=-I${DARWINDIR}/Linux/include/ -I${DARWINDIR}/Framework/include/ -I/usr/include/opencv2 -I/usr/include/eigen3 -g -std=c++0x -O3 -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

LDFLAGS=-lpthread -lrt `pkg-config opencv --libs`

TARGET=main

SOURCES=\
main.cc \
Agent/run.cc Agent/init.cc Agent/think.cc Agent/processImage.cc \
${VISIONDIR}/BlobDetector/detectBlobs.cc ${VISIONDIR}/BlobDetector/runLengthEncode.cc ${VISIONDIR}/BlobDetector/runSetToBlob.cc

HEADERS=Agent/agent.hh Agent/agent.ih ${VISIONDIR}/BlobDetector/blobdetector.hh

OBJECTS=${SOURCES:.cc=.o}

export

all: ${TARGET} darwin.a

clean:
	rm ${OBJECTS}

darwin.a:
	make -C ${DARWINDIR}/Linux/build

${OBJECTS}: ${HEADERS}

${TARGET}: darwin.a ${OBJECTS} ${HEADERS}
	${CXX} ${OBJECTS} ${DARWINDIR}/Linux/lib/darwin.a ${LDFLAGS} -o ${TARGET}
