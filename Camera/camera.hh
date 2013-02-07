#ifndef BOLD_CAMERA_HH
#define BOLD_CAMERA_HH

#include <string>
#include <vector>
#include <linux/videodev2.h>

namespace bold
{
  class Camera
  {
  public:

    enum ControlType
    {
      CT_INT = 1,
      CT_BOOL,
      CT_MENU,
      CT_INT_MENU,
      CT_BITMASK,
      CT_BUTTON,
      CT_INT64,
      CT_STRING,
      CT_CTRL_CLASS
    };

    struct ControlMenuItem
    {
      unsigned id;
      unsigned index;
      std::string name;
      int value;
    };

    struct Control
    {
      Control(v4l2_queryctrl const& qc)
	: id(qc.id),
	  type((ControlType)qc.type),
	  name((const char*)qc.name),
	  minimum(qc.minimum),
	  maximum(qc.maximum),
	  step(qc.step),
	  defaultValue(qc.default_value),
	  flags(qc.flags)
      {}

      unsigned id;
      ControlType type;
      std::string name;
      int minimum;
      int maximum;
      int step;
      int defaultValue;
      int value;
      unsigned flags;
      std::vector<ControlMenuItem> menuItems;
    };

  public:

    Camera(std::string const& device);

    void open();

    std::vector<Control> listControls();

    void getControlValue(Control &control);
    void setControlValue(Control const& control);

  private:
    std::string d_device;
    int d_fd;

    void fillControlMenuItems(Control& control);
  };
}

#endif
