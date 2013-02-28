#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../ImageLabeller/imagelabeller.hh"
#include "../LUTBuilder/lutbuilder.hh"

#include "../LineRunTracker/lineruntracker.hh"
#include "../HoughLine/houghline.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../HoughLineExtractor/houghlineextractor.hh"
#include "../ImagePasser/imagepasser.hh"
#include "../ImagePassHandler/LineDetectPass/linedetectpass.hh"

using namespace cv;
using namespace std;
using namespace bold;

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    cout <<" Usage: passtest <rgb-image>" << endl;
    return -1;
  }

  // Load the colour image
  cv::Mat colourImage = imread(argv[1], CV_LOAD_IMAGE_COLOR);

  if (!colourImage.data)
  {
    cout << "Could not open or find the image" << std::endl;
    return -1;
  }

  cout << "Reading " << argv[1] << endl;

  // Build colour ranges for segmentation
  Colour::hsvRange fieldRange = Colour::hsvRange(71, 20, 138, 55, 173, 65);
  Colour::hsvRange lineRange  = Colour::hsvRange(0, 255, 0, 70, 255, 70);
  Colour::hsvRange goalRange  = Colour::hsvRange(40, 10, 210, 55, 190, 65);
  Colour::hsvRange ballRange  = Colour::hsvRange(13, 30, 255, 95, 190, 95);

  vector<Colour::hsvRange> ranges;
  ranges.push_back(fieldRange);
  ranges.push_back(lineRange);
  ranges.push_back(goalRange);
  ranges.push_back(ballRange);

  // Label the image
  cv::Mat labelledImage(colourImage.size(), CV_8UC1);
  auto imageLabeller = new ImageLabeller(ranges);
  imageLabeller->label(colourImage, labelledImage);

  // Draw out the labelled image
  cv::Mat colourLabelledImage(colourImage.size(), CV_8UC3);
  ImageLabeller::colourLabels(labelledImage, colourLabelledImage, ranges);
  imwrite("labelled.jpg", colourLabelledImage);

  auto lineDetect = new LineDetectPass<uchar>(labelledImage.cols, labelledImage.rows, 50, 720, 1, 2, 1);

  vector<ImagePassHandler<uchar>*> handlers = { lineDetect };

  auto passer = ImagePasser<uchar>(handlers);

  // TODO provide the colour image, and have the passer use the LUT
  passer.pass(labelledImage);

  //
  // Write the accumulator image out to a file
  //
  cv::Mat accImg = lineDetect->accumulator.getMat().clone();
//cv::rectangle(accImg, Point(0,0), Point(accImg.cols-1, accImg.rows-1), Scalar(1));
  cv::normalize(accImg, accImg, 0, 255, NORM_MINMAX, CV_16UC1);
  imwrite("accumulator.jpg", accImg);

  auto lines = lineDetect->lines;

  //
  // Find lines
  //
  cout << "Found " << lines.size() << " line(s):" << endl;

  if (lines.size() == 0)
  {
    return 1;
  }

  double maxVotes = lines[0].votes();
  for (bold::HoughLine const& line : lines) {
    cout << "  theta=" << line.theta() << " (" << (line.theta()*180.0/M_PI) << " degs) radius=" << line.radius() << " m=" << line.gradient() << " c=" << line.yIntersection() << endl;
    Colour::bgr lineColor(0, 0, 255 * (line.votes()/maxVotes)); // red
    line.draw<Colour::bgr>(colourImage, lineColor);
  }

  imwrite("color-lines.jpg", colourImage);

  return 0;
}