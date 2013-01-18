CXXFLAGS=-I/usr/include/opencv2 -O0 -std=c++0x
LDFLAGS=-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lstdc++

opencvtest: opencvtest.cc
	$(CC) $(CXXFLAGS) opencvtest.cc -o $@ $(LDFLAGS)
