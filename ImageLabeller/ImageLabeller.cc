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
  vector<int> horizon(image.cols);
  int maxHorizon = -1;

  if (ignoreAboveHorizon)
  {
    for (int x = 0; x < image.cols; ++x)
    {
      int h = d_spatialiser->findHorizonForColumn(x);
      horizon[x] = h;
      if (h > maxHorizon)
        maxHorizon = h;
    }
  }

  
  for (int y = 0; y < image.rows; ++y)
  {
    uchar* origpix = image.ptr<uchar>(y);
    uchar* labelledpix = labelled.ptr<uchar>(y);

    if (ignoreAboveHorizon && y > maxHorizon)
    {
      memset(labelledpix, 0, image.cols);
      continue;
    }


    for (int x = 0; x < image.cols; ++x)
    {
//    uchar l = d_LUT[(origpix[0] << 16) | (origpix[1] << 8) | origpix[2]];
        uchar l =
          ignoreAboveHorizon && y > horizon[x] ?
          0 :
          lut[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];

      *labelledpix = l;

      ++origpix;
      ++origpix;
      ++origpix;
      ++labelledpix;
    }
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
