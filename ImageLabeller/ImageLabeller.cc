#include "imagelabeller.hh"

using namespace bold;
using namespace std;
using namespace cv;

ImageLabeller::ImageLabeller(std::vector<bold::hsvRange> ranges)
: d_LUT(LUTBuilder().buildBGR18FromHSVRanges(ranges))
{
  std::cout << "[ImageLabeller::ImageLabeller] Constructed" << std::endl;
}

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

void ImageLabeller::colourLabels(cv::Mat& labelledImage, cv::Mat& output, std::vector<hsvRange> const& ranges)
{
  std::map<uchar,Point3_<uchar>> colorByLabel;
  for (uchar label = 0; label < ranges.size(); label++)
  {
    hsvRange const& range = ranges.at(label);
    bgr bgr = LUTBuilder::hsv2bgr(hsv(range.h, range.s, range.v));
    colorByLabel[label + 1] = Point3_<uchar>(bgr.b, bgr.g, bgr.r);

    cout << "Label " << (int)label << " with HSV " << range.h << " " << range.s << " " << range.v << " has RGB " << (int)bgr.r << " " << (int)bgr.g << " " << (int)bgr.b << endl;
  }

  int count = 0;
  for (int y = 0; y < labelledImage.rows; y++)
  {
    for(int x = 0; x < labelledImage.cols; x++)
    {
      uchar label = labelledImage.at<uchar>(y,x);

      if (label > 0 && label <= ranges.size())
      {
        Point3_<uchar> bgr = colorByLabel[label];
        output.at<Point3_<uchar> >(y,x) = bgr;
        count++;
      }
    }
  }

  cout << count << " labelled pixels coloured" << endl;
}