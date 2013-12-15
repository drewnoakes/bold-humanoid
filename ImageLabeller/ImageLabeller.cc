#include "imagelabeller.hh"

#include "../SequentialTimer/sequentialtimer.hh"

#include <memory>

using namespace bold;
using namespace std;
using namespace cv;

ImageLabeller::ImageLabeller(shared_ptr<Spatialiser> spatialiser)
  : d_LUT(),
    d_spatialiser(spatialiser)
{}

ImageLabeller::ImageLabeller(shared_ptr<uchar const> const& lut, shared_ptr<Spatialiser> spatialiser)
  : d_LUT(lut),
    d_spatialiser(spatialiser)
{}

void ImageLabeller::label(Mat& image, Mat& labelled, SequentialTimer& timer, std::function<Eigen::Vector2i(int)> granularityFunction, bool ignoreAboveHorizon) const
{
  // TODO use the granularity function to avoid labelling pixels we won't use (?)

  uchar const* lut = d_LUT.get();

  // Everything above (and including) this row is guaranteed to be above horizon
  int maxAboveHorizonY;
  // Everything below this row is guaranteed to be under horizon
  int minHorizonY = image.rows;
  // The horizon's y level at the sides of the image. We assume it is a straight
  // line and interpolate linearly between these two values.
  int minXHorizonY, maxXHorizonY;

  // If we are ignoring everything above the horizon, find out where
  // the horizon is for each column.
  // Remember, y = 0 is bottom of field of view (the image is upside down)
  if (ignoreAboveHorizon)
  {
    minXHorizonY = d_spatialiser->findHorizonForColumn(0);
    maxXHorizonY = d_spatialiser->findHorizonForColumn(image.cols - 1);
    minHorizonY = min(minXHorizonY, maxXHorizonY);
    maxAboveHorizonY = max(minXHorizonY, maxXHorizonY);
  }
  else
  {
    minXHorizonY = image.rows;
    maxXHorizonY = image.rows;
    maxAboveHorizonY = image.rows;
  }

  timer.timeEvent("Find Horizon");

  ++maxAboveHorizonY;
  minHorizonY = max(0, minHorizonY);
  maxAboveHorizonY = min(image.rows, maxAboveHorizonY);

  // First batch: everything guaranteed under the horizon
  for (int y = 0; y < minHorizonY; ++y)
  {
    uchar* origpix = image.ptr<uchar>(y);
    uchar* labelledpix = labelled.ptr<uchar>(y);

    for (int x = 0; x < image.cols; ++x)
    {
      uchar l =
        lut[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];

      *labelledpix = l;

      ++origpix;
      ++origpix;
      ++origpix;
      ++labelledpix;
    }
  }

  timer.timeEvent("Pixels Under");

  if (ignoreAboveHorizon)
  {
    // Second batch: horizon goes through these rows
    int horizonYRange = maxAboveHorizonY - minHorizonY;
    for (int y = minHorizonY; y < maxAboveHorizonY; ++y)
    {
      uchar* origpix = image.ptr<uchar>(y);
      uchar* labelledpix = labelled.ptr<uchar>(y);

      double ratio = (y - minHorizonY) / (double)horizonYRange;
      int horizonY = Math::lerp(ratio, minXHorizonY, maxXHorizonY);

      for (int x = 0; x < image.cols; ++x)
      {
        uchar l =
          y > horizonY ?
          0 :
          lut[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];

        *labelledpix = l;

        ++origpix;
        ++origpix;
        ++origpix;
        ++labelledpix;
      }
    }

    timer.timeEvent("Pixels Around");

    // Third batch: everything here is above the horizon
    for (int y = maxAboveHorizonY; y < image.rows; ++y)
    {
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
