#include "datastreamer.ih"

void DataStreamer::streamImage(cv::Mat const& img, std::string const& imageEncoding, std::map<uchar, Colour::bgr> const& palette)
{
  ASSERT(ThreadUtil::isThinkLoopThread());

  lock_guard<mutex> guard(d_cameraSessionsMutex);

  for (auto ses : d_cameraSessions)
    ses->notifyImageAvailable(img, imageEncoding, palette);
}
