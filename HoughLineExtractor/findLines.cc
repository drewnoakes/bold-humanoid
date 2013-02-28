#include "houghlineextractor.hh"

#include <cassert>

using namespace bold;
using namespace std;

std::vector<HoughLine> HoughLineExtractor::findLines(HoughLineAccumulator& accumulator, int threshold, double angleSearch, int radiusSearch)
{
  assert(angleSearch != 0);
  assert(radiusSearch != 0);

  auto lines = std::vector<HoughLine>();

  int accumulatorWidth = accumulator.getMat().cols;
  int accumulatorHeight = accumulator.getMat().rows;
  int halfAccumulatorWidth = accumulatorWidth / 2;

  double thetaStepRadians = M_PI / accumulatorHeight;

  int angleSearchInt = (abs(angleSearch) / M_PI) * accumulatorHeight;

  // Loop through all angles
  for (int y = 0; y < accumulatorHeight; y++)
  {
    // Loop through all radii
    for (int x = 0; x < accumulatorWidth; x++)
    {
      ushort value = accumulator.getMat().at<ushort>(y, x);

      if (value <= threshold)
        continue;

      ushort peakValue = value;

      bool isLocalMaxima = true;
      for (int dx = -radiusSearch; isLocalMaxima && dx <= radiusSearch; dx++)
      {
        for (int dy = -angleSearchInt; dy <= angleSearchInt; dy++)
        {
          // coordinates of neighbour
          int nx = x + dx;
          int ny = y + dy;

          if (nx < 0 || nx >= accumulatorWidth)
          {
            continue;
          }

          // Consider symmetry and wrap around theta values when searching.
          // Note that when wrapping the angle, we have to negate the radius.
          if (ny < 0)
          {
            ny += accumulatorHeight;
            nx = ((nx - halfAccumulatorWidth) * -1) + halfAccumulatorWidth;
          }
          else if (ny >= accumulatorHeight)
          {
            ny -= accumulatorHeight;
            nx = ((nx - halfAccumulatorWidth) * -1) + halfAccumulatorWidth;
          }

          assert(ny >= 0 && ny < accumulatorHeight);
          assert(nx >= 0 && nx < accumulatorWidth);

          ushort neighbourValue = accumulator.getMat().at<ushort>(ny, nx);

          if (neighbourValue > peakValue)
          {
            isLocalMaxima = false;
            break;
          }
        }
      }

      if (isLocalMaxima)
      {
          // theta along the x axis, r along the y axis
          double theta = accumulator.getTheta(y);
          double radius = accumulator.getRadius(x);

          lines.push_back(HoughLine(radius, theta, peakValue));

          // If we found a local maximum, then the next 'radiusSearch' pixels
          // cannot be local maxima.
          x += radiusSearch - 1;
      }
    }
  }

  sort(lines.begin(), lines.end(), [](HoughLine a, HoughLine b) {
    // highest votes first
    return a.votes() > b.votes();
  });

  return lines;
}
