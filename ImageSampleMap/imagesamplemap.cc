#include "imagesamplemap.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

ImageSampleMap::ImageSampleMap(function<Matrix<uchar, 2, 1>(ushort)> granularityFunction, ushort width, ushort height)
  : d_granularities(),
    d_width(width)
{
  unsigned pixelCount = 0;
  ushort y = 0;
  while (y < height)
  {
    auto granularity = granularityFunction(y);
    y += granularity.y();
    pixelCount += (unsigned)ceil((double)d_width / granularity.x());
    d_granularities.emplace_back(move(granularity));
  }

  d_pixelCount = pixelCount;
}
