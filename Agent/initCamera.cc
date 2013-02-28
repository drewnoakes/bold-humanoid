#include "agent.ih"

void Agent::initCamera()
{
  cout << "[Agent::initCamera] Start" << endl;

  d_camera->open();

  auto controls = d_camera->getControls();

  cout << "===== CAPABILITIES =====" << endl;

  cout << "Read/write: " << (d_camera->canRead() ? "YES" : "NO") << endl;
  cout << "Streaming:  " << (d_camera->canStream() ? "YES" : "NO") << endl;

  cout << "===== CONTROLS =====" << endl;;
  for (auto control : controls)
  {
    if (control.name == "Brightness")
      control.setValue(50);

    cout << "Control: " << control.name << " " << control.type << " (" << control.minimum << "-" << control.maximum << ", def: " << control.defaultValue << "), val.: ";
    
    cout << control.getValue() << endl;
  }

  cout << "===== FORMATS =====" << endl;
  auto formats = d_camera->getFormats();
  for (auto format : formats)
  {
    cout << "Format: "  << format.description << endl;
  }

  cout << "===== CURRENT FORMAT =====" << endl;
  bool res = d_camera->getPixelFormat().requestSize(320,240);
  cout << "Set format: " << (res ? "YES" : "NO") << endl;

  auto pixelFormat = d_camera->getPixelFormat();


  cout << "Width          : " << pixelFormat.width << endl;
  cout << "Height         : " << pixelFormat.height << endl;
  cout << "Bytes per line : " << pixelFormat.bytesPerLine << endl;
  cout << "Bytes total    : " << pixelFormat.imageByteSize << endl;


  d_camera->startCapture();
}
