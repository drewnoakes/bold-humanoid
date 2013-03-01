#include "imagelabeller.hh"

using namespace bold;
using namespace std;
using namespace cv;

ImageLabeller::ImageLabeller(std::vector<Colour::hsvRange> ranges)
: d_LUT(LUTBuilder().buildBGR18FromHSVRanges(ranges))
{}

void ImageLabeller::label(cv::Mat& image, cv::Mat& labelled)
{
  for (unsigned y = 0; y < image.rows; ++y)
  {
    unsigned char *origpix = image.ptr<unsigned char>(y);
    unsigned char *labelledpix = labelled.ptr<unsigned char>(y);
    for (unsigned x = 0; x < image.cols; ++x)
    {
//    unsigned char l = d_LUT[(origpix[0] << 16) | (origpix[1] << 8) | origpix[2]];
      unsigned char l = d_LUT[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];
      *labelledpix = l;

      ++origpix;
      ++origpix;
      ++origpix;
      ++labelledpix;
    }
  }
}

void ImageLabeller::colourLabels(cv::Mat& labelledImage, cv::Mat& output, std::vector<Colour::hsvRange> const& ranges)
{
  std::map<uchar,Colour::bgr> colorByLabel;

  for (uchar label = 0; label < ranges.size(); label++)
  {
    colorByLabel[label + 1] = ranges.at(label).toBgr();
  }

  int count = 0;
  for (int y = 0; y < labelledImage.rows; y++)
  {
    for(int x = 0; x < labelledImage.cols; x++)
    {
      uchar label = labelledImage.at<uchar>(y,x);

      if (label > 0 && label <= ranges.size())
      {
        output.at<Colour::bgr >(y,x) = colorByLabel[label];
        count++;
      }
    }
  }

  cout << count << " labelled pixels coloured" << endl;
}
