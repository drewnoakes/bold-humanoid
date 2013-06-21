#include "camera.ih"

void Camera::createControls()
{
  auto getValue = [this](unsigned const& id) -> int
  {
    v4l2_control ctrl = {0,};
    ctrl.id = id;
    ioctl(d_fd, VIDIOC_G_CTRL, &ctrl);
    return ctrl.value;
  };

  auto setValue = [this,getValue](unsigned const& id, int const& value)
  {
    v4l2_control ctrl = {0,};
    ctrl.id = id;
    ctrl.value = value;
    ioctl(d_fd, VIDIOC_S_CTRL, &ctrl);

    // Test whether the value we set was taken or not
    int retrieved = getValue(id);

    if (retrieved != value)
      cerr << "[Camera::setValue] Setting camera control with ID " << id << " failed -- set " << value << " but read back " << retrieved << endl;
  };

  d_controls.clear();

  v4l2_queryctrl queryctrl = {0,};
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

    set<string> advancedControlNames = { "Auto WB", "Exposure, Auto", "Exposure, Auto Priority", "Backlight Compensation", "Power Line Frequency" };
    bool isAdvanced = advancedControlNames.find(name) != advancedControlNames.end();

    if (type == V4L2ControlType::CT_INT && minimum == 0 && maximum == 1)
    {
      type = V4L2ControlType::CT_BOOL;
    }

    switch (type)
    {
      case V4L2ControlType::CT_BOOL:
      {
        auto control = Control::createBool(
          id,
          name,
          [getValue,id]() { return getValue(id) != 0; },
          [setValue,id](bool const& value) { setValue(id, value ? 1 : 0); });
        control->setDefaultValue(defaultValue);
        control->setIsAdvanced(isAdvanced);
        d_controls.push_back(control);
        break;
      }
      case V4L2ControlType::CT_INT:
      {
        auto control = Control::createInt(
          id,
          name,
          [getValue,id]() { return getValue(id); },
          [setValue,id](int const& value) { setValue(id, value); });
        control->setDefaultValue(defaultValue);
        control->setLimitValues(minimum, maximum);
        control->setIsAdvanced(isAdvanced);
        d_controls.push_back(control);
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
          [getValue,id]() { return (unsigned)getValue(id); },
          [setValue,id](ControlEnumValue const& value) { setValue(id, value.getValue()); });
        control->setDefaultValue(defaultValue);
        control->setIsAdvanced(isAdvanced);
        d_controls.push_back(control);
        break;
      }
      default:
      {
        cerr << "Unsupported camera control type: " << (int)type << endl;
      }
    }
  }
}
