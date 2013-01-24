CXXFLAGS=-I/usr/include/opencv2 -I/usr/include/eigen3 -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -O3 -std=c++0x -g
LDFLAGS=`pkg-config opencv --libs`

BLOBDETECTOR_SOURCES=../BlobDetector/detectBlobs.cc ../BlobDetector/runLengthEncode.cc ../BlobDetector/runSetToBlob.cc

BLOBDETECTOR_OBJECTS=$(BLOBDETECTOR_SOURCES:.cc=.o)

all: djsettest blobtest

clean:
	rm ${BLOBDETECTOR_OBJECTS}
	rm blobtest

djsettest: djsettest.cc ../DisjointSet/disjointset.hh
	g++ djsettest.cc -o djsettest -O3 -std=c++0x

${BLOBDETECTOR_OBJECTS}: ../BlobDetector/blobdetector.hh ../DisjointSet/disjointset.hh

blobtest.o:  ../BlobDetector/blobdetector.hh ../DisjointSet/disjointset.hh

blobtest: blobtest.o ${BLOBDETECTOR_OBJECTS} 
	g++ blobtest.o ${CXXFLAGS} ${LDFLAGS} ${BLOBDETECTOR_OBJECTS} -o blobtest
