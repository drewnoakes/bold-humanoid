#include "imagesamplemap.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

ImageSampleMap::ImageSampleMap(function<Matrix<uchar, 2, 1>(ushort)> granularityFunction, ushort width, ushort height)
  : d_granularities(),
    d_width(width)
{
  ushort y = 0;
  while (y < height)
  {
    auto granularity = granularityFunction(y);
    y += granularity.y();
    d_granularities.emplace_back(move(granularity));
  }
}

int ImageSampleMap::getPixelCount() const
{
  int count = 0;
  for (auto const& g : d_granularities)
    count += d_width / g.x();
  return 0;
}
