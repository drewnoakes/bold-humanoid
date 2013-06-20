#include "agent.ih"

void Agent::initCamera()
{
  cout << "[Agent::initCamera] Start" << endl;

  d_camera = make_shared<Camera>("/dev/video0");

  d_camera->open();

  cout << "[Agent::initCamera] Capabilities:" << endl
       << "[Agent::initCamera]   Read/write: " << (d_camera->canRead() ? "YES" : "NO") << endl
       << "[Agent::initCamera]   Streaming:  " << (d_camera->canStream() ? "YES" : "NO") << endl;

  //
  // Set control values from config
  //

  cout << "[Agent::initCamera] Configuring camera from ini file" << endl;
  string sectionName = "Camera";
  for (Control& control : d_camera->getControls())
  {
    string name = control.getName();

    // strip strange characters from name
    char excludeChars[] = "()-, ";
    for (unsigned int i = 0; i < strlen(excludeChars); ++i)
      name.erase(remove(name.begin(), name.end(), excludeChars[i]), name.end());

    if (!paramExists(name))
    {
      cout << "[Agent::initCamera] No config key for '" << name << "'" << endl;
    }
    else
    {
      ControlType type = control.getType();
      switch (type) {
        case ControlType::Action:
        case ControlType::Unknown:
        default:
          break;
        case ControlType::Bool:
        case ControlType::Enum:
        case ControlType::Int:
          int value = getParam(name, control.getDefaultValue());
          control.setValue(value);
      }
    }
  }

  // HACK set some camera properties explicitly while we don't have a configuration system
  auto trySetCameraControl = [this](string name, int value)
  {
    Maybe<Control> c = d_camera->getControl(name);
    if (!c.hasValue())
    {
      cerr << "[Agent::initCamera] No camera control found for: " << name << endl;
    }
    else
    {
      (*c).setValue(value);

      // Test whether the value we set was taken or not
      int retrieved = (*c).getValue();

      if (retrieved != value)
        cerr << "[Agent::initCamera] Setting camera control '" << name << "' failed -- set " << value << " but read back " << retrieved << endl;
    }
  };
  trySetCameraControl("Auto WB", 0);
  trySetCameraControl("Exposure, Auto", 3);
  trySetCameraControl("Exposure, Auto Priority", 0);
  trySetCameraControl("Backlight Compensation", 0);
  trySetCameraControl("Exposure (Absolute)", 133);
  trySetCameraControl("Brightness", 128);
  trySetCameraControl("Contrast", 32);
  trySetCameraControl("Saturation", 28);
  trySetCameraControl("Gain", 255);
  trySetCameraControl("WB Temp (K)", 0);
  trySetCameraControl("Power Line Frequency", 2);
  trySetCameraControl("Sharpness", 191);

  cout << "[Agent::initCamera] Controls (" << d_camera->getControls().size() << "):" << endl;;
  for (Control const& control : d_camera->getControls())
    cout << "[Agent::initCamera]   " << control << endl;

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
