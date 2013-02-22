CXXFLAGS=-I/usr/include/opencv2 -I/usr/include/eigen3 -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -O3 -std=c++0x -g
LDFLAGS=`pkg-config opencv --libs`

BLOBDETECTOR_SOURCES=../BlobDetector/detectBlobs.cc ../BlobDetector/runLengthEncode.cc ../BlobDetector/runSetToBlob.cc
BLOBDETECTOR_OBJECTS=$(BLOBDETECTOR_SOURCES:.cc=.o)

all: djsettest blobtest linetest

djsettest: djsettest.cc ../DisjointSet/disjointset.hh
	g++ djsettest.cc -o djsettest -O3 -std=c++0x

${BLOBDETECTOR_OBJECTS}: ../BlobDetector/blobdetector.hh ../DisjointSet/disjointset.hh

blobtest.o:  ../BlobDetector/blobdetector.hh ../DisjointSet/disjointset.hh

blobtest: blobtest.o ${BLOBDETECTOR_OBJECTS}
	g++ blobtest.o ${CXXFLAGS} ${LDFLAGS} ${BLOBDETECTOR_OBJECTS} -o blobtest

CAMERA_SOURCES=\
	../Camera/Camera.cc\
	../Camera/open.cc\
	../Camera/listControls.cc\
	../Camera/getControlValue.cc\
	../Camera/setControlValue.cc\
	../Camera/listFormats.cc\
	../Camera/pixelFormatRequestSize.cc\
	../Camera/initMemoryMapping.cc\
	../Camera/startCapture.cc\
	../Camera/stopCapture.cc\
	../Camera/capture.cc\
	../PixelFilterChain/applyFilters.cc

CAMERA_OBJECTS=$(CAMERA_SOURCES:.cc=.o)

${CAMERA_OBJECTS} cameratest.o: ../Camera/camera.hh ../Camera/camera.ih

cameratest: cameratest.o ${CAMERA_OBJECTS}
	g++ cameratest.o ${CXXFLAGS} ${LDFLAGS} ${CAMERA_OBJECTS} -o cameratest

LINEDETECTOR_SOURCES=../HoughLineAccumulator/houghlineaccumulator.cc ../HoughLineExtractor/findLines.cc
LINEDETECTOR_OBJECTS=$(LINEDETECTOR_SOURCES:.cc=.o)

${LINEDETECTOR_OBJECTS}: ../HoughLineAccumulator/houghlineaccumulator.hh ../HoughLineExtractor/houghlineextractor.hh

linetest.o: ../HoughLineAccumulator/houghlineaccumulator.hh ../HoughLineExtractor/houghlineextractor.hh

linetest: linetest.o ${LINEDETECTOR_OBJECTS}
	g++ linetest.o ${CXXFLAGS} ${LDFLAGS} ${LINEDETECTOR_OBJECTS} -o linetest

clean:
	rm -f ${BLOBDETECTOR_OBJECTS}
	rm -f ${CAMERA_OBJECTS}
	rm -f ${LINEDETECTOR_OBJECTS}
	rm -f blobtest
	rm -f cameratest
	rm -f linetest

