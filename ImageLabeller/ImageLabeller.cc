#include "imagelabeller.hh"

#include "../SequentialTimer/sequentialtimer.hh"
#include "../Spatialiser/spatialiser.hh"
#include "../Math/math.hh"

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

void ImageLabeller::label(Mat const& image, Mat& labelled, SequentialTimer& timer, function<Vector2i(int)> granularityFunction, bool ignoreAboveHorizon) const
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

  int y = 0;
  Vector2i granularity;

  minHorizonY = min(minHorizonY, image.rows);

  // First batch: everything guaranteed under the horizon
  for (; y < minHorizonY; y += granularity.y())
  {
    granularity = granularityFunction(y);

    uchar const* origpix = image.ptr<uchar>(y);
    uchar* labelledpix = labelled.ptr<uchar>(y);

    for (int x = 0; x < image.cols; x += granularity.x())
    {
      uchar l =
        lut[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];

      *labelledpix = l;

      origpix += granularity.x() * 3;
      labelledpix += granularity.x();
    }
  }

  timer.timeEvent("Pixels Under");
  double horizonYRange = maxXHorizonY - minXHorizonY;
  if (ignoreAboveHorizon && minHorizonY < image.rows)
  {
    ++maxHorizonY;

    bool horizonUpwards = maxXHorizonY > minXHorizonY;

    // Second batch: horizon goes through these rows
    for (; y < maxHorizonY && y < image.rows; y += granularity.y())
    {
      granularity = granularityFunction(y);

      uchar const* origpix = image.ptr<uchar>(y);
      uchar* labelledpix = labelled.ptr<uchar>(y);

      double ratio = double(y - minXHorizonY) / double(horizonYRange);

      int horizonX = Math::lerp(ratio, 0, image.cols - 1);
      // TODO: wrap two loops in for loop? Our Atom can't do branch
      // prediction, so doing same check inside loop may be costly
      // We can also directly fill the line we know to be 0
      for (int x = 0; x < image.cols; x += granularity.x())
      {
        bool aboveHorizon =
          (horizonUpwards && x < horizonX) ||
          (!horizonUpwards && x > horizonX);

        uchar l =
          aboveHorizon ?
          0 :
          lut[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];

        *labelledpix = l;

        origpix += granularity.x() * 3;
        labelledpix += granularity.x();
      }
    }

    timer.timeEvent("Pixels Around");

    // Third batch: everything here is above the horizon
    for (; y < image.rows; y += granularity.y())
    {
      granularity = granularityFunction(y);

      uchar* labelledpix = labelled.ptr<uchar>(y);
      memset(labelledpix, 0, image.cols);
    }

    timer.timeEvent("Pixels Above");
  }
}

void ImageLabeller::createCartoon(Mat& labelledInput, Mat& cartoonOutput, vector<bold::PixelLabel> const& labels)
{
  map<uchar,Colour::bgr> colorByLabel;

  for (PixelLabel const& label : labels)
  {
    colorByLabel[label.id()] = label.hsvRange().toBgr();
  }

  for (int y = 0; y < labelledInput.rows; y++)
  {
    uchar* in = labelledInput.ptr<uchar>(y);
    Colour::bgr* out = cartoonOutput.ptr<Colour::bgr>(y);

    for(int x = 0; x < labelledInput.cols; x++)
    {
      uchar labelId = *(in++);

      if (labelId != 0)
      {
        auto it = colorByLabel.find(labelId);
        if (it != colorByLabel.end())
        {
          *out = it->second;
        }
      }

      out++;
    }
  }
}
