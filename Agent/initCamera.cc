#include "agent.ih"

void Agent::initCamera()
{
  cout << "[Agent::initCamera] Start" << endl;

  // TODO get camera height/width from config
  unsigned width = 320;
  unsigned height = 240;

  d_camera->open();

  cout << "===== CAPABILITIES =====" << endl;

  cout << "Read/write: " << (d_camera->canRead() ? "YES" : "NO") << endl;
  cout << "Streaming:  " << (d_camera->canStream() ? "YES" : "NO") << endl;

  vector<string> confControls = {"Brightness"};

  for (string const& controlName : confControls)
  {
    auto control = d_camera->getControl(controlName);
    if (control)
      control->setValue(d_ini.geti("Camera", controlName, control->getValue()));
  }

  cout << "===== CONTROLS =====" << endl;;
  for (Control const& control : d_camera->getControls())
  {
    cout << "Control: " << control << endl;
  }

  cout << "===== FORMATS =====" << endl;
  for (auto const& format : d_camera->getFormats())
    cout << "Format: "  << format.description << endl;

  cout << "===== CURRENT FORMAT =====" << endl;
  bool res = d_camera->getPixelFormat().requestSize(width, height);
  cout << "Request size " << width << "x" << height << ": " << (res ? "OK" : "FAIL") << endl;

  auto pixelFormat = d_camera->getPixelFormat();
  cout << "Width          : " << pixelFormat.width << endl;
  cout << "Height         : " << pixelFormat.height << endl;
  cout << "Bytes per line : " << pixelFormat.bytesPerLine << endl;
  cout << "Bytes total    : " << pixelFormat.imageByteSize << endl;

  d_camera->startCapture();
}
