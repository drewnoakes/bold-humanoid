#include "agent.ih"

void Agent::initCamera()
{
  cout << "[Agent::initCamera] Start" << endl;

  d_camera->open();

  cout << "[Agent::initCamera] Capabilities:" << endl
       << "[Agent::initCamera]   Read/write: " << (d_camera->canRead() ? "YES" : "NO") << endl
       << "[Agent::initCamera]   Streaming:  " << (d_camera->canStream() ? "YES" : "NO") << endl;

  vector<string> confControls = {"Brightness"};

  for (string const& controlName : confControls)
  {
    auto control = d_camera->getControl(controlName);
    if (control)
      control->setValue(d_ini.geti("Camera", controlName, control->getValue()));
  }

  cout << "[Agent::initCamera] Controls (" << d_camera->getControls().size() << "):" << endl;;
  for (Control const& control : d_camera->getControls())
  {
    cout << "[Agent::initCamera]   " << control << endl;
  }

  cout << "[Agent::initCamera] Formats (" << d_camera->getFormats().size() << "):" << endl;;
  for (Camera::Format const& format : d_camera->getFormats())
    cout << "[Agent::initCamera]   "  << format.description << endl;

  unsigned width = d_cameraModel->imageWidth();
  unsigned height = d_cameraModel->imageHeight();
  bool res = d_camera->getPixelFormat().requestSize(width, height);
  cout << "[Agent::initCamera] Requesting size " << width << "x" << height << ": " << (res ? "OK" : "FAIL") << endl;

  auto pixelFormat = d_camera->getPixelFormat();
  cout << "[Agent::initCamera] Current format:" << endl;;
  cout << "[Agent::initCamera]   Width          : " << pixelFormat.width << endl;
  cout << "[Agent::initCamera]   Height         : " << pixelFormat.height << endl;
  cout << "[Agent::initCamera]   Bytes per line : " << pixelFormat.bytesPerLine << endl;
  cout << "[Agent::initCamera]   Bytes total    : " << pixelFormat.imageByteSize << endl;

  d_camera->startCapture();
}
