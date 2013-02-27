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

class LineRunTracker
{
private:
  enum class State : unsigned char { In, On, Out };

  unsigned char inLabel; // eg: green
  unsigned char onLabel; // eg: white
  State state;
  unsigned short int otherCoordinate;
  unsigned short int startedAt;
  std::function<void(unsigned short int const, unsigned short int const, unsigned short int const)> callback;
  unsigned hysterisis;
  unsigned hysterisisLimit;

public:
  LineRunTracker(
    unsigned char inLabel,
    unsigned char onLabel,
    unsigned short int otherCoordinate,
    unsigned char hysterisisLimit,
    std::function<void(unsigned short int const, unsigned short int const, unsigned short int const)> callback
  )
  : inLabel(inLabel),
    onLabel(onLabel),
    otherCoordinate(otherCoordinate),
    callback(callback),
    hysterisisLimit(hysterisisLimit),
    hysterisis(0),
    state(State::Out)
  {}

  void reset()
  {
    state = State::Out;
  }

  void update(unsigned char label, unsigned short int position)
  {
    switch (state)
    {
      case State::Out:
      {
        if (label == inLabel)
        {
          hysterisis = 0;
          state = State::In;
        }
        break;
      }
      case State::In:
      {
        if (label == onLabel)
        {
          state = State::On;
          hysterisis = 0;
          startedAt = position;
        }
        else if (label == inLabel)
        {
          if (hysterisis != hysterisisLimit)
            hysterisis++;
        }
        else
        {
          if (hysterisis != 0)
            hysterisis--;
          else
            state = State::Out;
        }
        break;
      }
      case State::On:
      {
        if (label == inLabel)
        {
          // we completed a run!
          state = State::In;
          hysterisis = 0;
          callback(startedAt, position, otherCoordinate);
        }
        else if (label == onLabel)
        {
          if (hysterisis != hysterisisLimit)
            hysterisis++;
        }
        else
        {
          if (hysterisis != 0)
            hysterisis--;
          else
            state = State::Out;
        }
        break;
      }
    }
  }
};

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

  // Build colour ranges for segmentation
  hsvRange fieldRange = hsvRange(71, 20, 138, 55, 173, 65);
  hsvRange lineRange  = hsvRange(0, 255, 0, 70, 255, 70);
  hsvRange goalRange  = hsvRange(40, 10, 210, 55, 190, 65);
  hsvRange ballRange  = hsvRange(13, 30, 255, 95, 190, 95);

  vector<hsvRange> ranges;
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

  // Allocate an image for line dots
  cv::Mat lineDotImage(labelledImage.size(), CV_8UC1);

  int lineDotCount = 0;

  //
  // Process labelled pixels
  //

  vector<LineRunTracker> colTrackers;
  for (unsigned x = 0; x <= labelledImage.cols; ++x)
  {
    colTrackers.push_back(LineRunTracker(1, 2, x, 3, [&](unsigned short int const from, unsigned short int const to, unsigned short int const other) {
      int mid = (from + to) / 2;
      lineDotImage.at<uchar>(mid, (int)other)++;
      lineDotCount++;
    }));
  }

  for (unsigned y = 0; y < labelledImage.rows; ++y)
  {
    unsigned char const* row = labelledImage.ptr<unsigned char>(y);

    auto rowTracker = LineRunTracker(1, 2, y, 3, [&](unsigned short int const from, unsigned short int const to, unsigned short int const other) {
      int mid = (from + to) / 2;
      lineDotImage.at<uchar>((int)other, mid)++;
      lineDotCount++;
    });

    // We go one pixel outside of the row, as if image is padded with a column of zeros
    for (unsigned x = 0; x <= labelledImage.cols; ++x)
    {
      unsigned char label = x < labelledImage.cols ? row[x] : 0;

      rowTracker.update(label, x);

      colTrackers[x].update(label, y);
    }
  }

  cout << "Found " << lineDotCount << " line dots" << endl;

  cv::normalize(lineDotImage, lineDotImage, 0, 255, NORM_MINMAX, CV_8UC1);
  imwrite("line-dots.jpg", lineDotImage);

  return 0;
}