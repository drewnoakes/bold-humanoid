CXXFLAGS=-I/usr/include/opencv2 -I/usr/include/eigen3 -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -O3 -std=c++0x -g
LDFLAGS=`pkg-config opencv --libs`

all: unittests

SOURCES=\
	../Camera/Camera.cc\
	../Camera/capture.cc\
	../Camera/getControlValue.cc\
	../Camera/fillControlMenuItems.cc\
	../Camera/initMemoryMapping.cc\
	../Camera/listControls.cc\
	../Camera/listFormats.cc\
	../Camera/open.cc\
	../Camera/pixelFormatRequestSize.cc\
	../Camera/setControlValue.cc\
	../Camera/startCapture.cc\
	../Camera/stopCapture.cc\
	../Colour/bgr2hsv.cc\
	../Colour/hsv2bgr.cc\
	../Colour/hsvRange.cc\
	../geometry/Bounds2i.cc\
	../geometry/LineSegment2i.cc\
	../HoughLineAccumulator/houghlineaccumulator.cc\
	../HoughLineExtractor/findLines.cc\
	../HoughLineExtractor/findMaxLine.cc\
	../PixelFilterChain/applyFilters.cc\
	../LUTBuilder/buildBGRFromHSVRanges.cc\
	../ImageLabeller/ImageLabeller.cc\
	../ImagePassHandler/BlobDetectPass/detectBlobs.cc\
	../ImagePassHandler/BlobDetectPass/runSetToBlob.cc\
	../PixelLabel/pixellabel.cc

HEADERS=\
	../Colour/colour.hh\
	../Camera/camera.hh\
	../Camera/camera.ih\
	../geometry/Bounds2i.hh\
	../geometry/Line.hh\
	../geometry/LineSegment2i.hh\
	../HoughLineExtractor/houghlineextractor.hh\
	../HoughLineAccumulator/houghlineaccumulator.hh\
	../HoughLineExtractor/houghlineextractor.hh\
	../ImageLabeller/imagelabeller.hh\
	../ImagePasser/imagepasser.hh\
	../ImagePassHandler/imagepasshandler.hh\
	../ImagePassHandler/BlobDetectPass/blobdetectpass.hh\
	../ImagePassHandler/LineDotPass/linedotpass.hh\
	../ImagePassHandler/CartoonPass/cartoonpass.hh\
	../ImagePassHandler/LabelCountPass/labelcountpass.hh\
	../LineFinder/linefinder.hh\
	../LUTBuilder/lutbuilder.hh

OBJECTS=$(SOURCES:.cc=.o)

${OBJECTS}: ${HEADERS}


# ----------------------------

gtest:
	cd google-test && cmake . && make

unittests: gtest UnitTests.cc ${OBJECTS}
	g++ UnitTests.cc ${OBJECTS} google-test/libgtest.a -Igoogle-test/include/ -isystem google-test/include/ -Igoogle-test ${CXXFLAGS} ${LDFLAGS} -lpthread -o unittests
	./unittests

# ----------------------------

clean:
	rm -f ${OBJECTS}
	rm -f UnitTests.o
	rm -f unittests

