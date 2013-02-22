#include "houghlineextractor.hh"

using namespace std;

std::vector<bold::HoughLine> bold::HoughLineExtractor::findLines(bold::HoughLineAccumulator& accumulator)
{
  auto lines = std::vector<bold::HoughLine>();

  cv::Mat mat = accumulator.getMat();

  uint16_t* ptr = reinterpret_cast<uint16_t*>(mat.data);

  //
  // Find maximum in entire accumulator
  //
  unsigned int maxY = 0, maxX = 0;
  uint16_t maxCount = 0;
  for (unsigned int y = 0; y < mat.rows; y++)
  {
    for (unsigned int x = 0; x < mat.cols; x++)
    {
      uint16_t value = *ptr;

      if (value > maxCount) {
        maxCount = value;
        maxX = x;
        maxY = y;
      }

      ptr++;
    }
  }

  // accumulator has theta in x, radius in y

  double radius = accumulator.getRadius(maxX);
  double theta = accumulator.getTheta(maxY);

  auto line = bold::HoughLine(radius, theta);

  lines.push_back(line);

  return lines;
}