#include "datastreamer.ih"

DataStreamer::DataStreamer(int port)
: d_image(),
  d_imageType(ImageType::RGB),
  d_streamFramePeriod(10),
  d_shouldDrawBlobs(true),
  d_shouldDrawLineDots(false),
  d_shouldDrawExpectedLines(true),
  d_shouldDrawObservedLines(true),
  d_camera(0),
//   d_dirtyStateProtocols(),
  d_cameraSessions(),
  d_controlsByIdByFamily(),
  d_port(port),
  d_context(0)
{}
