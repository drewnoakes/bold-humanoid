#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "../HoughLine/houghline.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../HoughLineExtractor/houghlineextractor.hh"

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
  // create an accumulator for a specific input domain
  auto accumulator = bold::HoughLineAccumulator(320, 240);

//  for (int i = 0; i < 1000*1000; i++) {
//    accumulator.add(i%320, i%240);
//  }

  accumulator.add( 0,  0);
  accumulator.add(10, 10);
  accumulator.add(20, 20);
  accumulator.add(30, 30);
  accumulator.add(40, 40);

  //
  // Write the accumulator image out to a file
  //
  cv::Mat accImg = accumulator.getMat().clone();
  cv::rectangle(accImg, Point(0,0), Point(accImg.cols-1, accImg.rows-1), Scalar(1));
  cv::normalize(accImg, accImg, 0, 255, NORM_MINMAX, CV_16UC1);
  imwrite("accumulator.jpg", accImg);

  //
  // Find lines
  //
  auto extractor = bold::HoughLineExtractor();

  std::vector<bold::HoughLine> lines = extractor.findLines(accumulator);

  cout << "Found " << lines.size() << " line(s):" << endl;

  for (bold::HoughLine const& line : lines) {
    cout << " theta=" << line.theta() << " (" << (line.theta()*180.0/M_PI) << " degs) radius=" << line.radius() << " m=" << line.gradient() << " c=" << line.yIntersection() << endl;
  }

  return 0;
}