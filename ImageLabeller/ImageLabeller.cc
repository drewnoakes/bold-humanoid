#include "imagelabeller.hh"

using namespace bold;
using namespace std;
using namespace cv;

ImageLabeller::ImageLabeller(uchar const * const lut)
: d_LUT(lut)
{}

void ImageLabeller::label(cv::Mat& image, cv::Mat& labelled)
{
  for (unsigned y = 0; y < image.rows; ++y)
  {
    uchar *origpix = image.ptr<uchar>(y);
    uchar *labelledpix = labelled.ptr<uchar>(y);
    for (unsigned x = 0; x < image.cols; ++x)
    {
//    uchar l = d_LUT[(origpix[0] << 16) | (origpix[1] << 8) | origpix[2]];
      uchar l = d_LUT[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];
      *labelledpix = l;

      ++origpix;
      ++origpix;
      ++origpix;
      ++labelledpix;
    }
  }
}

void ImageLabeller::colourLabels(cv::Mat& labelledImage, cv::Mat& output, std::vector<PixelLabel> const& labels)
{
  std::map<uchar,Colour::bgr> colorByLabel;

  for (PixelLabel const& label : labels)
  {
    colorByLabel[label.id()] = label.hsvRange().toBgr();
  }

  int count = 0;
  for (int y = 0; y < labelledImage.rows; y++)
  {
    for(int x = 0; x < labelledImage.cols; x++)
    {
      uchar labelId = labelledImage.at<uchar>(y, x);

      if (labelId == 0)
      {
        continue;
      }

      auto it = colorByLabel.find(labelId);
      if (it != colorByLabel.end())
      {
        output.at<Colour::bgr>(y,x) = it->second;
        count++;
      }
    }
  }

  cout << count << " labelled pixels coloured" << endl;
}
