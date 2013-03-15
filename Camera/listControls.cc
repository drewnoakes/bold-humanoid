#include "camera.ih"

vector<Control> Camera::listControls()
{
  auto setValue = [this](unsigned const& id, int const& value)
  {
    v4l2_control ctrl;
    ctrl.id = id;
    ctrl.value = value;
    ioctl(d_fd, VIDIOC_S_CTRL, &ctrl);
  };

  auto getValue = [this](unsigned const& id) -> int
  {
    v4l2_control ctrl;
    ctrl.id = id;
    ioctl(d_fd, VIDIOC_G_CTRL, &ctrl);
    return ctrl.value;
  };

  vector<Control> controls;

  v4l2_queryctrl queryctrl;
  queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

  while (ioctl(d_fd, VIDIOC_QUERYCTRL, &queryctrl) == 0)
  {
    // Ignore disabled controls
    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
      continue;

    unsigned id = queryctrl.id;
    V4L2ControlType type = (V4L2ControlType)queryctrl.type;
    std::string name = (const char*)queryctrl.name;
    int minimum = queryctrl.minimum;
    int maximum = queryctrl.maximum;
    int defaultValue = queryctrl.default_value;
    int value = getValue(id);

    // Get ready to query next item
    queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;

    // Remove some we cannot use
    if (name == "Pan (Absolute)" || name == "Tilt (Absolute)")
      continue;

    // Rename some verbose items
    if (name == "White Balance Temperature, Auto")
      name = "Auto WB";
    else if (name == "White Balance Temperature")
      name = "WB Temp (K)";

    switch (type)
    {
      case V4L2ControlType::CT_BOOL:
      {
        auto control = Control::createBool(
          id,
          name,
          value != 0,
          [setValue,id](bool const& value) { setValue(id, value ? 1 : 0); });
        control.setDefaultValue(defaultValue);
        controls.push_back(control);
        break;
      }
      case V4L2ControlType::CT_INT:
      {
        auto control = Control::createInt(
          id,
          name,
          value,
          [setValue,id](int const& value) { setValue(id, value); });
        control.setDefaultValue(defaultValue);
        control.setLimitValues(minimum, maximum);
        controls.push_back(control);
        break;
      }
      case V4L2ControlType::CT_MENU:
      {
        struct v4l2_querymenu querymenu;
        memset (&querymenu, 0, sizeof(querymenu));
        querymenu.id = id;

        // Query all enum values
        vector<ControlEnumValue> enumValues;
        for (unsigned i = minimum; i <= maximum; i++)
        {
          querymenu.index = i;

          if (ioctl(d_fd, VIDIOC_QUERYMENU, &querymenu) == 0)
          {
            enumValues.push_back(ControlEnumValue(
              querymenu.index,
              string((const char*)querymenu.name)));
          }
        }

        auto control = Control::createEnum(
          id,
          name,
          enumValues,
          (unsigned)value,
          [setValue,id](ControlEnumValue const& value) { setValue(id, value.getValue()); });
        control.setDefaultValue(defaultValue);
        controls.push_back(control);
        break;
      }
      default:
      {
        cerr << "Unsupported camera control type: " << (int)type << endl;
      }
    }
  }

  return controls;
}
