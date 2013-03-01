CXXFLAGS=-I/usr/include/opencv2 -I/usr/include/eigen3 -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -O3 -std=c++0x -g
LDFLAGS=`pkg-config opencv --libs`

all: djsettest hough passtest

djsettest: djsettest.cc ../DisjointSet/disjointset.hh
	g++ djsettest.cc -o djsettest -O3 -std=c++0x

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

LINEDETECTOR_SOURCES=\
	../HoughLineAccumulator/houghlineaccumulator.cc\
	../HoughLineExtractor/findLines.cc\
	../HoughLineExtractor/findMaxLine.cc
LINEDETECTOR_OBJECTS=$(LINEDETECTOR_SOURCES:.cc=.o)
${LINEDETECTOR_OBJECTS}: ../HoughLineAccumulator/houghlineaccumulator.hh ../HoughLineExtractor/houghlineextractor.hh

hough.o: ../HoughLineAccumulator/houghlineaccumulator.hh ../HoughLineExtractor/houghlineextractor.hh ../HoughLine/houghline.hh

hough: hough.o ${LINEDETECTOR_OBJECTS}
	g++ hough.o ${CXXFLAGS} ${LDFLAGS} ${LINEDETECTOR_OBJECTS} -o hough

PASS_SOURCES=\
	../Colour/bgr2hsv.cc\
	../Colour/hsv2bgr.cc\
	../LUTBuilder/buildBGRFromHSVRanges.cc\
	../ImageLabeller/ImageLabeller.cc\
	../ImagePassHandler/BlobDetectPass/detectBlobs.cc\
	../ImagePassHandler/BlobDetectPass/runSetToBlob.cc
PASS_OBJECTS=$(PASS_SOURCES:.cc=.o)
${PASS_OBJECTS}: ../LUTBuilder/lutbuilder.hh ../ImageLabeller/imagelabeller.hh ../Colour/colour.hh ../ImagePassHandler/imagepasshandler.hh ../ImagePassHandler/BlobDetectPass/blobdetectpass.hh ../ImagePassHandler/LineDetectPass/linedetectpass.hh

passtest: passtest.o hough.o ${LINEDETECTOR_OBJECTS} ${PASS_OBJECTS}
	g++ passtest.o ${CXXFLAGS} ${LDFLAGS} ${LINEDETECTOR_OBJECTS} ${PASS_OBJECTS} -o passtest

clean:
	rm -f ${PASS_OBJECTS}
	rm -f ${CAMERA_OBJECTS}
	rm -f ${LINEDETECTOR_OBJECTS}
	rm -f ${LUT_OBJECTS}
	rm -f cameratest
	rm -f djsettest
	rm -f hough
	rm -f hough.o
	rm -f passtest
	rm -f passtest.o

