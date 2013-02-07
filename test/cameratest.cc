#include "../Camera/camera.hh"
#include <iostream>

using namespace bold;
using namespace std;


int main()
{
  Camera cam("/dev/video0");

  cam.open();

  auto controls = cam.listControls();

  
  for (auto control : controls)
  {
    if (control.name == "Brightness")
    {
      control.value = 50;
      cam.setControlValue(control);
    }

    cout << "Control: " << control.name << " " << control.type << " (" << control.minimum << "-" << control.maximum << ", def: " << control.defaultValue << "), val.: ";
    cam.getControlValue(control);
    cout << control.value << endl;
  }

}
