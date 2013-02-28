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

/**
 * A state machine that calls back when it detects a run of one label sandwiched
 * between runs of another label.
 *
 * This type was designed for finding line segments in an image row/column.
 *
 * Hysterisis is used to address both noise and gaps at borders between the
 * two labels being tracked.
 */
class LineRunTracker
{
private:
  enum class State : uchar { In, On, Out };

  uchar inLabel; // eg: green
  uchar onLabel; // eg: white
  State state;
  ushort startedAt;
  std::function<void(ushort const, ushort const, ushort const)> callback;
  unsigned hysterisis;
  unsigned hysterisisLimit;

public:
  ushort otherCoordinate;

  LineRunTracker(
    uchar inLabel,
    uchar onLabel,
    ushort otherCoordinate,
    uchar hysterisisLimit,
    std::function<void(ushort const, ushort const, ushort const)> callback
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

  void update(uchar label, ushort position)
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
        else
        {
          if (hysterisis != 0)
            hysterisis--;
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

  auto accumulator = bold::HoughLineAccumulator(labelledImage.cols, labelledImage.rows);

  const uchar inLabel = 1;
  const uchar onLabel = 2;
  const uchar hysterisisLimit = 1; // TODO learn/control this variable (from config at least)

  vector<LineRunTracker> colTrackers;
  for (int x = 0; x <= labelledImage.cols; ++x)
  {
    colTrackers.push_back(LineRunTracker(
      inLabel, onLabel, /*otherCoordinate*/x, hysterisisLimit,
      [&](ushort const from, ushort const to, ushort const other) {
        int mid = (from + to) / 2;
        lineDotImage.at<uchar>(mid, (int)other)++;
        accumulator.add((int)other, (int)mid);
        lineDotCount++;
      }
    ));
  }

  auto rowTracker = LineRunTracker(
    inLabel, onLabel, /*otherCoordinate*/0, hysterisisLimit,
    [&](ushort const from, ushort const to, ushort const other) {
      int mid = (from + to) / 2;
      lineDotImage.at<uchar>((int)other, mid)++;
      accumulator.add((int)mid, (int)other);
      lineDotCount++;
    }
  );

  for (int y = 0; y < labelledImage.rows; ++y)
  {
    uchar const* row = labelledImage.ptr<uchar>(y);

    rowTracker.reset();
    rowTracker.otherCoordinate = y;

    LineRunTracker* colTracker = colTrackers.data();

    // We go one pixel outside of the row, as if image is padded with a column of zeros
    for (int x = 0; x != labelledImage.cols; ++x)
    {
      uchar label = x == labelledImage.cols ? 0 : row[x];

      rowTracker.update(label, x);

      colTracker->update(label, y);
      colTracker++;
    }
  }

  cout << "Found " << lineDotCount << " line dots" << endl;

  cv::normalize(lineDotImage, lineDotImage, 0, 255, NORM_MINMAX, CV_8UC1);
  imwrite("line-dots.jpg", lineDotImage);

  //
  // Write the accumulator image out to a file
  //
  cv::Mat accImg = accumulator.getMat().clone();
//cv::rectangle(accImg, Point(0,0), Point(accImg.cols-1, accImg.rows-1), Scalar(1));
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