#include "agent.ih"

void Agent::initCamera()
{
  d_camera = make_shared<Camera>(Config::getStaticValue<string>("hardware.video-path"));

  d_camera->open();

  //
  // Set control values from config
  //

  cout << "[Agent::initCamera] Configuring camera from ini file" << endl;
  string sectionName = "Camera";
  for (shared_ptr<Camera::Control const> control : d_camera->getControls())
  {
    string name = control->name;

    // strip strange characters from name
    char excludeChars[] = "()-, ";
    for (unsigned int i = 0; i < strlen(excludeChars); ++i)
      name.erase(remove(name.begin(), name.end(), excludeChars[i]), name.end());

    // TODO SETTINGS reinstate camera control settings

//     if (!paramExists(name))
//     {
//       cout << "[Agent::initCamera] No config key for '" << name << "'" << endl;
//     }
//     else
//     {
//       ControlType type = control->getType();
//       switch (type) {
//         case ControlType::Action:
//         case ControlType::Unknown:
//         default:
//           break;
//         case ControlType::Bool:
//         case ControlType::Enum:
//         case ControlType::Int:
//           int value = getParam(name, control->getDefaultValue());
//           control->setValue(value);
//       }
//     }
  }

  // HACK set some camera properties explicitly while we don't have a configuration system
  auto trySetCameraControl = [this](string name, int value)
  {
    shared_ptr<Camera::Control const> c = d_camera->getControl(name);
    if (!c)
      cerr << ccolor::error << "[Agent::initCamera] No camera control found for: " << name << ccolor::reset << endl;
    // TODO SETTINGS
//     else
//       c->setValue(value);
  };
  trySetCameraControl("Auto WB", 0); // off
  trySetCameraControl("Exposure, Auto", 1); // manual
  trySetCameraControl("Exposure, Auto Priority", 0); // off
  trySetCameraControl("Backlight Compensation", 0);
  trySetCameraControl("Power Line Frequency", 2);

  trySetCameraControl("Exposure (Absolute)", 1000);
  trySetCameraControl("Brightness", 128);
  trySetCameraControl("Contrast", 32);
  trySetCameraControl("Saturation", 28);
  trySetCameraControl("Gain", 128);
  trySetCameraControl("WB Temp (K)", 0);
  trySetCameraControl("Sharpness", 191);

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
