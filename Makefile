CXX=g++

DARWINDIR=robotis
VISIONDIR=vision
JOYSTICKDIR=joystick

CXXFLAGS=-I${DARWINDIR}/Linux/include/ -I${DARWINDIR}/Framework/include/ -I${VISIONDIR} -I${JOYSTICKDIR} -I/usr/include/opencv2 -I/usr/include/eigen3 -J2 -O2 -g -std=c++0x -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

LDFLAGS=-lpthread -lrt `pkg-config opencv --libs`

TARGET=main

SOURCES=\
main.cc \
Agent/Agent.cc Agent/run.cc Agent/init.cc Agent/think.cc Agent/processImage.cc Debugger/debugger.cc \
${VISIONDIR}/BlobDetector/detectBlobs.cc ${VISIONDIR}/BlobDetector/runLengthEncode.cc ${VISIONDIR}/BlobDetector/runSetToBlob.cc \
${VISIONDIR}/LUTBuilder/bgr2hsv.cc ${VISIONDIR}/LUTBuilder/buildBGRFromHSVRanges.cc \
${JOYSTICKDIR}/joystick.cc

HEADERS=Agent/agent.hh Agent/agent.ih Ambulator/ambulator.hh Debugger/debugger.hh ${VISIONDIR}/BlobDetector/blobdetector.hh ${VISIONDIR}/LUTBuilder/lutbuilder.hh ${JOYSTICKDIR}/joystick.hh

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
