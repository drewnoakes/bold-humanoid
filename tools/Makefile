CXXFLAGS=-I/usr/include/opencv2 -I/usr/include/eigen3 -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -O3 -std=c++0x -g
LDFLAGS=`pkg-config opencv --libs`


all: djsettest hough passtest cameratest


GEOMETRY_SOURCES=\
	../geometry/Bounds2i.cc\
	../geometry/LineSegment2i.cc
GEOMETRY_OBJECTS=$(GEOMETRY_SOURCES:.cc=.o)
GEOMETRY_HEADERS=\
	../geometry/Bounds2i.hh\
	../geometry/Line.hh\
	../geometry/LineSegment2i.hh\
	../geometry/LineSegment3d.hh
${GEOMETRY_OBJECTS}: ${GEOMETRY_HEADERS}



CAMERA_SOURCES=\
	../Camera/Camera.cc\
	../Camera/capture.cc\
	../Camera/initMemoryMapping.cc\
	../Camera/fillControlMenuItems.cc\
	../Camera/getControlValue.cc\
	../Camera/listControls.cc\
	../Camera/listFormats.cc\
	../Camera/open.cc\
	../Camera/pixelFormatRequestSize.cc\
	../Camera/setControlValue.cc\
	../Camera/startCapture.cc\
	../Camera/stopCapture.cc\
	../PixelFilterChain/applyFilters.cc
CAMERA_OBJECTS=$(CAMERA_SOURCES:.cc=.o)
${CAMERA_OBJECTS} cameratest.o: ../Camera/camera.hh ../Camera/camera.ih



LINEDETECTOR_SOURCES=\
	../HoughLineAccumulator/houghlineaccumulator.cc\
	../HoughLineExtractor/findLines.cc\
	../HoughLineExtractor/findMaxLine.cc
LINEDETECTOR_OBJECTS=$(LINEDETECTOR_SOURCES:.cc=.o)
${LINEDETECTOR_OBJECTS}: ../HoughLineAccumulator/houghlineaccumulator.hh ../HoughLineExtractor/houghlineextractor.hh



PASS_SOURCES=\
	../Colour/bgr2hsv.cc\
	../Colour/hsv2bgr.cc\
	../Colour/hsvRange.cc\
	../ImageLabeller/ImageLabeller.cc\
	../ImagePassHandler/BlobDetectPass/detectBlobs.cc\
	../ImagePassHandler/BlobDetectPass/runSetToBlob.cc\
	../HoughLineAccumulator/houghlineaccumulator.cc\
	../HoughLineExtractor/findLines.cc\
	../HoughLineExtractor/findMaxLine.cc\
	../LineFinder/find.cc\
	../LUTBuilder/buildBGRFromHSVRanges.cc\
	../PixelLabel/pixellabel.cc
PASS_OBJECTS=$(PASS_SOURCES:.cc=.o)
${PASS_OBJECTS} passtest.o: ../LUTBuilder/lutbuilder.hh ../ImageLabeller/imagelabeller.hh ../Colour/colour.hh ../ImagePasser/imagepasser.hh ../ImagePassHandler/imagepasshandler.hh ../ImagePassHandler/BlobDetectPass/blobdetectpass.hh ../ImagePassHandler/LineDotPass/linedotpass.hh ../HoughLineAccumulator/houghlineaccumulator.hh ../HoughLineExtractor/houghlineextractor.hh ../ImagePassHandler/CartoonPass/cartoonpass.hh ../ImagePassHandler/LabelCountPass/labelcountpass.hh ../LineFinder/linefinder.hh




djsettest: djsettest.cc ../DisjointSet/disjointset.hh
	g++ djsettest.cc -o djsettest -O3 -std=c++0x


passtest.o:                 ../LUTBuilder/lutbuilder.hh ../ImageLabeller/imagelabeller.hh ../Colour/colour.hh ../ImagePasser/imagepasser.hh ../ImagePassHandler/imagepasshandler.hh ../ImagePassHandler/BlobDetectPass/blobdetectpass.hh ../ImagePassHandler/LineDotPass/linedotpass.hh ../HoughLineAccumulator/houghlineaccumulator.hh ../HoughLineExtractor/houghlineextractor.hh ../ImagePassHandler/CartoonPass/cartoonpass.hh ../ImagePassHandler/LabelCountPass/labelcountpass.hh ../LineFinder/linefinder.hh
passtest: passtest.o hough.o ${PASS_OBJECTS} ${GEOMETRY_OBJECTS}
	g++ passtest.o ${CXXFLAGS} ${LDFLAGS} ${PASS_OBJECTS} ${GEOMETRY_OBJECTS} -o passtest


cameratest: cameratest.o ${CAMERA_OBJECTS} ${GEOMETRY_OBJECTS}
	g++ cameratest.o ${CXXFLAGS} ${LDFLAGS} ${CAMERA_OBJECTS} ${GEOMETRY_OBJECTS} -o cameratest


hough.o: ../HoughLineAccumulator/houghlineaccumulator.hh ../HoughLineExtractor/houghlineextractor.hh
hough: hough.o ${LINEDETECTOR_OBJECTS}
	g++ hough.o ${CXXFLAGS} ${LDFLAGS} ${LINEDETECTOR_OBJECTS} -o hough

# ----------------------------

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
