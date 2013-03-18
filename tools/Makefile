CXXFLAGS=-I/usr/include/opencv2 -I/usr/include/eigen3 -I../rapidjson/include -I../robotis/Linux/include -I../robotis/Framework/include -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -O3 -std=c++0x -g
LDFLAGS=`pkg-config opencv --libs`


all: djsettest hough passtest cameratest


SOURCES=\
	../geometry/Bounds2i.cc\
	../geometry/LineSegment2i.cc\
	../Camera/Camera.cc\
	../Camera/capture.cc\
	../Camera/initMemoryMapping.cc\
	../Camera/getControlValue.cc\
	../Camera/listControls.cc\
	../Camera/listFormats.cc\
	../Camera/open.cc\
	../Camera/pixelFormatRequestSize.cc\
	../Camera/startCapture.cc\
	../Camera/stopCapture.cc\
	../Control/create.cc\
	../Colour/bgr2hsv.cc\
	../Colour/hsv2bgr.cc\
	../Colour/hsvRange.cc\
	../Colour/YCbCr.cc\
	../Control/handleRequest.cc\
	../Control/staticfields.cc\
	../Control/writeState.cc\
	../HoughLineAccumulator/houghlineaccumulator.cc\
	../HoughLineExtractor/findLines.cc\
	../HoughLineExtractor/findMaxLine.cc\
	../ImageLabeller/ImageLabeller.cc\
	../ImagePassHandler/BlobDetectPass/BlobDetectPass.cc\
	../ImagePassHandler/BlobDetectPass/detectBlobs.cc\
	../ImagePassHandler/BlobDetectPass/runSetToBlob.cc\
	../LineFinder/MaskWalkLineFinder/findLineSegments.cc\
	../LineFinder/MaskWalkLineFinder/initialise.cc\
	../LineFinder/MaskWalkLineFinder/MaskWalkLineFinder.cc\
	../LineFinder/MaskWalkLineFinder/rebuild.cc\
	../LineFinder/RandomPairLineFinder/findLineSegments.cc\
	../LUTBuilder/buildLookUpTable.cc\
	../PixelFilterChain/applyFilters.cc\
	../PixelLabel/pixellabel.cc
OBJECTS=$(SOURCES:.cc=.o)
HEADERS=\
	../geometry/Bounds.hh\
	../geometry/Bounds2i.hh\
	../geometry/Line.hh\
	../geometry/LineSegment.hh\
	../geometry/LineSegment2i.hh\
	../Camera/camera.hh\
	../Camera/camera.ih\
	../Colour/colour.hh\
	../HoughLineAccumulator/houghlineaccumulator.hh\
	../HoughLineExtractor/houghlineextractor.hh\
	../ImageLabeller/imagelabeller.hh\
	../ImagePassRunner/imagepassrunner.hh\
	../ImagePassHandler/imagepasshandler.hh\
	../ImagePassHandler/CartoonPass/cartoonpass.hh\
	../ImagePassHandler/BlobDetectPass/blobdetectpass.hh\
	../ImagePassHandler/LineDotPass/linedotpass.hh\
	../ImagePassHandler/LabelCountPass/labelcountpass.hh\
	../LineFinder/MaskWalkLineFinder/maskwalklinefinder.hh\
	../LineFinder/RandomPairLineFinder/randompairlinefinder.hh\
	../LUTBuilder/lutbuilder.hh
${OBJECTS}: ${HEADERS}



djsettest: djsettest.cc ../DisjointSet/disjointset.hh
	g++ djsettest.cc -o djsettest -O3 -std=c++0x


passtest.o: ${HEADERS}
passtest: passtest.o ${OBJECTS}
	g++ passtest.o ${CXXFLAGS} ${LDFLAGS} ${OBJECTS} -o passtest


cameratest.o: ${HEADERS}
cameratest: cameratest.o ${OBJECTS}
	g++ cameratest.o ${CXXFLAGS} ${LDFLAGS} ${OBJECTS} -o cameratest


hough.o: ${HEADERS}
hough: hough.o ${OBJECTS}
	g++ hough.o ${CXXFLAGS} ${LDFLAGS} ${OBJECTS} -o hough

# ----------------------------

clean:
	rm -f ${OBJECTS}
	rm -f cameratest
	rm -f djsettest
	rm -f hough
	rm -f hough.o
	rm -f passtest
	rm -f passtest.o
