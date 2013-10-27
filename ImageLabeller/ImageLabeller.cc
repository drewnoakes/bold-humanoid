#include "imagelabeller.hh"

#include <memory>

using namespace bold;
using namespace std;
using namespace cv;

ImageLabeller::ImageLabeller(shared_ptr<uchar> const& lut, shared_ptr<Spatialiser> spatialiser)
  : d_LUT(lut),
    d_spatialiser(spatialiser)
{}

void ImageLabeller::label(Mat& image, Mat& labelled, bool ignoreAboveHorizon) const
{
  uchar* lut = d_LUT.get();
  vector<int> horizonYAtCol(image.cols);

  // Everything above this row is guaranteed to be above horizon
  int minAboveHorizonY = -1;
  // Everything below this row is guaranteed to be under horizon
  int minHorizonY = image.rows;

  // If we are ignoring everything above the horizon, find out where
  // the horizon is for each column
  // remember: y = 0 is bottom of field of view
  // TODO: is horizon a straight line?  If so, then just determining
  // outer columns is enough
  if (ignoreAboveHorizon)
  {
    for (int x = 0; x < image.cols; ++x)
    {
      int h = d_spatialiser->findHorizonForColumn(x);
      horizonYAtCol[x] = h;
      if (h > minAboveHorizonY)
        minAboveHorizonY = h;
      if (h < minHorizonY)
        minHorizonY = h;
    }
  }
  else
    minAboveHorizonY = image.rows;

  ++minAboveHorizonY;
  minHorizonY = min(image.rows, minHorizonY);
  minAboveHorizonY = min(image.rows, minAboveHorizonY);

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

  // Second batch: horizon goes through these rows
  // Never get into this batch if ignoreAboveHorizon == false
  for (int y = minHorizonY; y < minAboveHorizonY; ++y)
  {
    uchar* origpix = image.ptr<uchar>(y);
    uchar* labelledpix = labelled.ptr<uchar>(y);

    for (int x = 0; x < image.cols; ++x)
    {
        uchar l =
          y > horizonYAtCol[x] ?
          0 :
          lut[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];

      *labelledpix = l;

      ++origpix;
      ++origpix;
      ++origpix;
      ++labelledpix;
    }
  }

  // Third batch: everything here is above the horizon
  for (int y = minAboveHorizonY; y < image.rows; ++y)
  {
    uchar* labelledpix = labelled.ptr<uchar>(y);
    memset(labelledpix, 0, image.cols);
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
