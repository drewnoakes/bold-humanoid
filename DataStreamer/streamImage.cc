#include "datastreamer.ih"

void DataStreamer::streamImage(Mat const& img, string imageEncoding)
{
  ASSERT(ThreadUtil::isThinkLoopThread());

  {
    lock_guard<mutex> imageGuard(d_cameraImageMutex);
    d_image = img;
    d_imageEncoding = imageEncoding;
  }

  lock_guard<mutex> guard(d_cameraSessionsMutex);

  for (auto ses : d_cameraSessions)
    ses->notifyImageAvailable();
}
