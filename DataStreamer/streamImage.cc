#include "datastreamer.ih"

void DataStreamer::streamImage(cv::Mat const& img)
{
  d_img = img;
  libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::CAMERA]);
}
