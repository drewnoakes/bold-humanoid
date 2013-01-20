CXXFLAGS=-I/usr/include/opencv2 -I/usr/include/eigen3 -O3 -g -std=c++0x
LDFLAGS= -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lstdc++

BLOBDETECTOR_SOURCES=../BlobDetector/detectBlobs.cc ../BlobDetector/runLengthEncode.cc ../BlobDetector/runSetToBlob.cc

BLOBDETECTOR_OBJECTS=$(BLOBDETECTOR_SOURCES:.cc=.o)

all: djsettest blobtest

djsettest: djsettest.cc ../DisjointSet/disjointset.hh
	g++ djsettest.cc -o djsettest -O3 -std=c++0x

${BLOBDETECTOR_OBJECTS}: ../BlobDetector/blobdetector.hh ../DisjointSet/disjointset.hh

blobtest.o:  ../BlobDetector/blobdetector.hh ../DisjointSet/disjointset.hh

blobtest: blobtest.o ${BLOBDETECTOR_OBJECTS} 
	g++ blobtest.o ${CXXFLAGS} ${LDFLAGS} ${BLOBDETECTOR_OBJECTS} -o blobtest
