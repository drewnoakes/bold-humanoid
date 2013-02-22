CXX=g++

DARWINDIR=robotis
VISIONDIR=vision
JOYSTICKDIR=joystick
LIBWEBSOCKETSDIR=libwebsockets

CXXFLAGS=-I${DARWINDIR}/Linux/include/ -I${DARWINDIR}/Framework/include/ -I${VISIONDIR} -I${JOYSTICKDIR} -I/usr/include/opencv2 -I/usr/include/eigen3 `pkg-config --cflags sigc++-2.0` -O2 -g -std=c++0x -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

LDFLAGS=-lpthread -lrt `pkg-config opencv --libs` `pkg-config --cflags --libs sigc++-2.0` -lwebsockets

TARGET=main

SOURCES=\
	main.cc \
	Agent/Agent.cc \
	Agent/run.cc \
	Agent/init.cc \
	Agent/think.cc \
	Agent/processImage.cc \
	Agent/processInputCommands.cc \
	Agent/standUpIfFallen.cc \
	Agent/lookForBall.cc \
	Agent/lookAtBall.cc \
	Agent/approachBall.cc \
	Agent/circleBall.cc \
	Agent/lookForGoal.cc \
	Agent/lookAtGoal.cc \
	Agent/lookAt.cc \
	Agent/preKickLook.cc \
	Agent/readSubBoardData.cc \
	DataStreamer/DataStreamer.cc \
	Debugger/debugger.cc \
	CM730Snapshot/init.cc \
	MX28Snapshot/init.cc \
	GameController/GameControllerReceiver.cc \
	${VISIONDIR}/BlobDetector/detectBlobs.cc \
	${VISIONDIR}/BlobDetector/runLengthEncode.cc \
	${VISIONDIR}/BlobDetector/runSetToBlob.cc \
$	${VISIONDIR}/LUTBuilder/bgr2hsv.cc \
	${VISIONDIR}/LUTBuilder/buildBGRFromHSVRanges.cc \
	${VISIONDIR}/Camera/Camera.cc \
	${VISIONDIR}/Camera/open.cc \
	${VISIONDIR}/Camera/listControls.cc \
	${VISIONDIR}/Camera/getControlValue.cc \
	${VISIONDIR}/Camera/setControlValue.cc \
	${VISIONDIR}/Camera/listFormats.cc \
	${VISIONDIR}/Camera/pixelFormatRequestSize.cc \
	${VISIONDIR}/Camera/initMemoryMapping.cc \
	${VISIONDIR}/Camera/startCapture.cc \
	${VISIONDIR}/Camera/stopCapture.cc \
	${VISIONDIR}/Camera/capture.cc \
	${VISIONDIR}/PixelFilterChain/applyFilters.cc \
	${JOYSTICKDIR}/joystick.cc

HEADERS=Agent/agent.hh Agent/agent.ih Ambulator/ambulator.hh DataStreamer/datastreamer.hh Debugger/debugger.hh ${VISIONDIR}/BlobDetector/blobdetector.hh ${VISIONDIR}/LUTBuilder/lutbuilder.hh ${JOYSTICKDIR}/joystick.hh ${VISIONDIR}/Camera/camera.hh GameController/GameControllerReceiver.hh GameController/RoboCupGameControlData.h CM730Snapshot/CM730Snapshot.hh MX28Snapshot/MX28Snapshot.hh

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
