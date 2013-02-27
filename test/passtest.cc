#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../ImageLabeller/imagelabeller.hh"
#include "../LUTBuilder/lutbuilder.hh"

#include "../HoughLine/houghline.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../HoughLineExtractor/houghlineextractor.hh"

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

  cv::Mat colourImage = imread(argv[1], CV_LOAD_IMAGE_COLOR);

  if (!colourImage.data)
  {
    cout << "Could not open or find the image" << std::endl;
    return -1;
  }

  hsvRange fieldRange = hsvRange(71, 20, 138, 55, 173, 65);
  hsvRange lineRange  = hsvRange(0, 255, 0, 70, 255, 70);
  hsvRange goalRange  = hsvRange(40, 10, 210, 55, 190, 65);
  hsvRange ballRange  = hsvRange(13, 30, 255, 95, 190, 95);

  vector<hsvRange> ranges;
  ranges.push_back(fieldRange);
  ranges.push_back(lineRange);
  ranges.push_back(goalRange);
  ranges.push_back(ballRange);

  cv::Mat labelledImage(colourImage.size(), CV_8UC1);

  auto imageLabeller = new ImageLabeller(ranges);
  imageLabeller->label(colourImage, labelledImage);

  cv::Mat colourLabelledImage(colourImage.size(), CV_8UC3);
  ImageLabeller::colourLabels(labelledImage, colourLabelledImage, ranges);

  imwrite("labelled.jpg", colourLabelledImage);

  return 0;
}