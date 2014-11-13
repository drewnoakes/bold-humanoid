#include "imagelabeller.hh"

#include "../ImageLabelData/imagelabeldata.hh"
#include "../ImageSampleMap/imagesamplemap.hh"
#include "../SequentialTimer/sequentialtimer.hh"
#include "../Spatialiser/spatialiser.hh"

using namespace cv;
using namespace bold;
using namespace Eigen;
using namespace std;

ImageLabeller::ImageLabeller(shared_ptr<Spatialiser> spatialiser)
  : d_LUT(),
    d_spatialiser(spatialiser),
    d_lutMutex()
{}

ImageLabeller::ImageLabeller(shared_ptr<uchar const> const& lut, shared_ptr<Spatialiser> spatialiser)
  : d_LUT(lut),
    d_spatialiser(spatialiser),
    d_lutMutex()
{}

void ImageLabeller::updateLut(shared_ptr<uchar const> const& lut)
{
  lock_guard<mutex> guard(d_lutMutex);
  d_LUT = lut;
}

ImageLabelData ImageLabeller::label(Mat const& image, ImageSampleMap const& sampleMap, bool ignoreAboveHorizon, SequentialTimer& timer) const
{
  // Make a threadsafe copy of the shared ptr, in case another thread reassigns the LUT (avoids segfault)
  unique_lock<mutex> guard(d_lutMutex);
  auto lutPtr = d_LUT;
  guard.unlock();

  uchar const* lut = lutPtr.get();

  // Everything above (and including) this row is guaranteed to be above horizon
  int maxHorizonY = 0;
  // Everything below this row is guaranteed to be under horizon
  int minHorizonY;
  // The horizon's y level at the sides of the image. We assume it is a straight
  // line and interpolate linearly between these two values.
  int minXHorizonY = 0, maxXHorizonY = 0;

  // If we are ignoring everything above the horizon, find out where
  // the horizon is for each column.
  // Remember, y = 0 is bottom of field of view (the image is upside down)
  if (ignoreAboveHorizon)
  {
    minXHorizonY = d_spatialiser->findHorizonForColumn(0);
    maxXHorizonY = d_spatialiser->findHorizonForColumn(image.cols - 1);
    minHorizonY = min(minXHorizonY, maxXHorizonY);
    maxHorizonY = max(minXHorizonY, maxXHorizonY);
  }
  else
  {
    // If we shouldn't ignore above horizon, just say it's at the top of the image
    minHorizonY = image.rows;
  }

  timer.timeEvent("Find Horizon");

  auto granularity = sampleMap.begin();
  ushort y = 0;

  minHorizonY = min(minHorizonY, image.rows);

  vector<RowLabels> rows;
  rows.reserve(sampleMap.getSampleRowCount());

  //
  // First batch: everything guaranteed under the horizon
  //

  while (y < minHorizonY)
  {
    uchar const* origpix = image.ptr<uchar>(y);

    vector<uchar> labels;
    labels.reserve(image.cols / granularity->x());

    for (int x = 0; x < image.cols; x += granularity->x())
    {
      uchar l = lut[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];
      labels.push_back(l);
      origpix += granularity->x() * 3;
    }

    rows.emplace_back(move(labels), y, *granularity);

    y += granularity->y();
    granularity++;
  }

  timer.timeEvent("Rows Under Horizon");

  //
  // Second batch: horizon goes through these rows
  //

  double horizonYRange = maxXHorizonY - minXHorizonY;
  if (ignoreAboveHorizon && minHorizonY < image.rows)
  {
    ++maxHorizonY;

    bool horizonUpwards = maxXHorizonY > minXHorizonY;

    while(y < maxHorizonY && y < image.rows)
    {
      uchar const* origpix = image.ptr<uchar>(y);
      vector<uchar> labels;
      labels.reserve(image.cols / granularity->x());

      double ratio = double(y - minXHorizonY) / double(horizonYRange);

      int horizonX = Math::lerp(ratio, 0, image.cols - 1);
      // TODO: wrap two loops in for loop? Our Atom can't do branch
      // prediction, so doing same check inside loop may be costly
      // We can also directly fill the line we know to be 0
      for (int x = 0; x < image.cols; x += granularity->x())
      {
        bool aboveHorizon =
          (horizonUpwards && x < horizonX) ||
          (!horizonUpwards && x > horizonX);

        uchar l = aboveHorizon
          ? 0
          : lut[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];

        labels.push_back(l);

        origpix += granularity->x() * 3;
      }

      y += granularity->y();
      granularity++;
    }

    timer.timeEvent("Rows Intersecting Horizon");
  }

  return ImageLabelData(move(rows));
}
