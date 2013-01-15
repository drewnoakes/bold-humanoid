CXXFLAGS=-I/usr/include/opencv2 -I/usr/include/eigen3 -O2 -g -std=c++0x
LDFLAGS=-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lstdc++

all: djsettest blobtest

djsettest: djsettest.cc ../DisjointSet/disjointset.hh
	g++ djsettest.cc -o djsettest -O3 -std=c++0x

blobtest: blobtest.cc ../DisjointSet/disjointset.hh ../BlobDetector/blobdetector.hh ../BlobDetector/detectBlobs.cc ../BlobDetector/runLengthEncode.cc
	g++ ${CXXFLAGS} blobtest.cc ../BlobDetector/detectBlobs.cc ../BlobDetector/runLengthEncode.cc -o blobtest -O3 -std=c++0x ${LDFLAGS} 
