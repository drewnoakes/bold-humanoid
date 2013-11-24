#include "agent.ih"

void Agent::initCamera()
{
  d_camera = make_shared<Camera>(Config::getStaticValue<string>("hardware.video-path"));

  d_camera->open();

//   cout << "[Agent::initCamera] Capabilities:" << endl
//        << "[Agent::initCamera]   Read/write: " << (d_camera->canRead() ? "YES" : "NO") << endl
//        << "[Agent::initCamera]   Streaming:  " << (d_camera->canStream() ? "YES" : "NO") << endl;
//
//   cout << "[Agent::initCamera] Controls (" << d_camera->getControls().size() << "):" << endl;;
//   for (std::shared_ptr<Camera::Control const> control : d_camera->getControls())
//     cout << "[Agent::initCamera]   " << control->name << endl;
//
//   cout << "[Agent::initCamera] Formats (" << d_camera->getFormats().size() << "):" << endl;;
//   for (Camera::Format const& format : d_camera->getFormats())
//     cout << "[Agent::initCamera]   "  << format.description << endl;

  unsigned width = d_cameraModel->imageWidth();
  unsigned height = d_cameraModel->imageHeight();
  bool res = d_camera->getPixelFormat().requestSize(width, height);

  if (!res)
    cerr << ccolor::error << "[Agent::initCamera] Requesting camera size " << width << "x" << height << " failed" << ccolor::reset << endl;

  auto pixelFormat = d_camera->getPixelFormat();
  cout << "[Agent::initCamera] Current format:" << endl;;
  cout << "[Agent::initCamera]   Width          : " << pixelFormat.width << endl;
  cout << "[Agent::initCamera]   Height         : " << pixelFormat.height << endl;
  cout << "[Agent::initCamera]   Bytes per line : " << pixelFormat.bytesPerLine << endl;
  cout << "[Agent::initCamera]   Bytes total    : " << pixelFormat.imageByteSize << endl;

  d_camera->startCapture();
}
