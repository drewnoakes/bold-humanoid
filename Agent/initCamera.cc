#include "agent.ih"

void Agent::initCamera()
{
  d_camera = make_shared<Camera>(Config::getStaticValue<string>("hardware.video-path"));

  d_camera->open();

  log::verbose("Agent::initCamera") << "Capabilities:";
  log::verbose("Agent::initCamera") << "  Read/write: " << (d_camera->canRead() ? "YES" : "NO");
  log::verbose("Agent::initCamera") << "  Streaming:  " << (d_camera->canStream() ? "YES" : "NO");

  log::verbose("Agent::initCamera") << "Controls (" << d_camera->getControls().size() << "):";
  for (shared_ptr<Camera::Control const> control : d_camera->getControls())
    log::verbose("Agent::initCamera") << "  " << control->name;

  log::verbose("Agent::initCamera") << "Formats (" << d_camera->getFormats().size() << "):";
  for (Camera::Format const& format : d_camera->getFormats())
    log::verbose("Agent::initCamera") << "  "  << format.description;

  unsigned width = d_cameraModel->imageWidth();
  unsigned height = d_cameraModel->imageHeight();
  bool res = d_camera->getPixelFormat().requestSize(width, height);

  if (!res)
    log::error() << "[Agent::initCamera] Requesting camera size " << width << "x" << height << " failed";

  auto pixelFormat = d_camera->getPixelFormat();
  log::info("Agent::initCamera") << "Current format:";
  log::info("Agent::initCamera") << "  Width          : " << pixelFormat.width;
  log::info("Agent::initCamera") << "  Height         : " << pixelFormat.height;
  log::info("Agent::initCamera") << "  Bytes per line : " << pixelFormat.bytesPerLine;
  log::info("Agent::initCamera") << "  Bytes total    : " << pixelFormat.imageByteSize;

  d_camera->logFrameIntervalDetails();

  d_camera->startCapture();
}
