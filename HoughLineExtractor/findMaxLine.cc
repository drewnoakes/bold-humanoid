#include "houghlineextractor.hh"

using namespace bold;
using namespace cv;
using namespace std;

Line HoughLineExtractor::findMaxLine(HoughLineAccumulator& accumulator) const
{
  Mat mat = accumulator.getMat();

  uint16_t* ptr = reinterpret_cast<uint16_t*>(mat.data);

  //
  // Find maximum in entire accumulator
  //
  unsigned int maxY = 0, maxX = 0;
  uint16_t maxVotes = 0;
  for (auto y = 0; y < mat.rows; y++)
  {
    for (auto x = 0; x < mat.cols; x++)
    {
      uint16_t value = *ptr;

      if (value > maxVotes) {
        maxVotes = value;
        maxX = x;
        maxY = y;
      }

      ptr++;
    }
  }

  // accumulator has theta in x, radius in y

  double radius = accumulator.getRadius(maxX);
  double theta = accumulator.getTheta(maxY);

  return Line(radius, theta, maxVotes);
}
