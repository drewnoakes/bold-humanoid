//#define NDEBUG

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>

#include "../geometry/Line.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../HoughLineExtractor/houghlineextractor.hh"
#include "../LUTBuilder/lutbuilder.hh"
#include "../ImagePassHandler/imagepasshandler.hh"
#include "../ImagePassHandler/HoughLinePass/houghlinepass.hh"
#include "../ImagePassRunner/imagepassrunner.hh"

using namespace cv;
using namespace std;
using namespace bold;

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    cout << "Detect lines in a grayscale image. Only non-zero pixels are considered." << endl;
    cout << endl;
    cout << " Usage: hough <rgb-image>" << endl;
    return -1;
  }

  // Load the colour image
  cv::Mat image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

  if (!image.data)
  {
    cout << "Could not open or find the image" << endl;
    return -1;
  }

  cout << "Processing image with " << image.cols << " cols and " << image.rows << " rows" << endl;

  auto hough = new HoughLinePass<uchar>(image.cols, image.rows, 80, 720);

  cout << "Accumulator has " << hough->accumulator.getMat().cols << " cols and " << hough->accumulator.getMat().rows << " rows" << endl;

  vector<ImagePassHandler<uchar>*> handlers = { hough };

  auto passer = ImagePassRunner<uchar>(handlers);

  passer.pass(image);

  //
  // Write the accumulator image out to a file
  //
  cv::Mat accImg = hough->accumulator.getMat().clone();
  cv::normalize(accImg, accImg, 0, 255, NORM_MINMAX, CV_16UC1);
  imwrite("accumulator.jpg", accImg);

  //
  // Find lines
  //
  cout << "Found " << hough->lines.size() << " line(s):" << endl;

  if (hough->lines.size() == 0)
  {
    return 1;
  }

  Mat colorLines;
  cvtColor(image, colorLines, CV_GRAY2BGR);

  double maxVotes = hough->lines[0].votes();
  for (Candidate<Line> const& line : hough->lines)
  {
    Colour::bgr lineColor(0, 0, 255 * (line.votes()/maxVotes)); // red
    line.item().draw(colorLines, lineColor.toScalar());
  }

  imwrite("color-lines.jpg", colorLines);

  return 0;
}