#include "datastreamer.ih"

void DataStreamer::streamImage(Mat const& img, string imageEncoding)
{
  d_image = img;
  d_imageEncoding = imageEncoding;

  for (auto ses : d_cameraSessions)
    ses->notifyImageAvailable();
}
