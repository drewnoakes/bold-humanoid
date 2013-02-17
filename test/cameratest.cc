#include "../Camera/camera.hh"
#include <iostream>

using namespace bold;
using namespace std;


int main()
{
  Camera cam("/dev/video0");

  cam.open();

  auto controls = cam.getControls();

  cout << "===== CAPABILITIES =====" << endl;

  cout << "Read/write: " << (cam.canStream() ? "YES" : "NO") << endl;
  cout << "Streaming:  " << (cam.canStream() ? "YES" : "NO") << endl;

  cout << "===== CONTROLS =====" << endl;;
  for (auto control : controls)
  {
    if (control.name == "Brightness")
      control.setValue(50);

    cout << "Control: " << control.name << " " << control.type << " (" << control.minimum << "-" << control.maximum << ", def: " << control.defaultValue << "), val.: ";
    
    cout << control.getValue() << endl;
  }

  cout << "===== FORMATS =====" << endl;
  auto formats = cam.getFormats();
  for (auto format : formats)
  {
    cout << "Format: "  << format.description << endl;
  }

  cout << "===== CURRENT FORMAT =====" << endl;
  cam.getPixelFormat().requestSize(640,400);
  auto pixelFormat = cam.getPixelFormat();


  cout << "Width          : " << pixelFormat.width << endl;
  cout << "Height         : " << pixelFormat.height << endl;
  cout << "Bytes per line : " << pixelFormat.height << endl;
  cout << "Bytes total    : " << pixelFormat.imageByteSize << endl;
}
