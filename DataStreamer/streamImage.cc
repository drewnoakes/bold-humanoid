#include "datastreamer.ih"

void DataStreamer::streamImage(cv::Mat const& img)
{
  d_image = img;

  for (auto ses : d_cameraSessions)
    if (!ses->imgSending)
      ses->imgReady = true;
}
