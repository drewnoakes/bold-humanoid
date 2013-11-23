#include "camera.ih"

#include <cctype>

void Camera::createControls()
{
  d_controls.clear();
  d_settings.clear();

  //
  // Query the device for all camera controls
  //

  v4l2_queryctrl queryctrl = {0,};
  queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

  while (ioctl(d_fd, VIDIOC_QUERYCTRL, &queryctrl) == 0)
  {
    // Ignore disabled controls
    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
      continue;

    d_controls.push_back(make_shared<Control>(queryctrl));

    // Get ready to query next item
    queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
  }

  //
  // Create Setting<T> objects for the camera controls
  //

  // NOTE unlike settings defined in configuration-metadata.json, these are not
  // necessarily known before the process starts, and will vary based upon the
  // available hardware.

  set<string> advancedControlNames = { "Auto WB", "Exposure, Auto", "Exposure, Auto Priority", "Backlight Compensation", "Power Line Frequency" };
  set<string> ignoreControlNames = { "Pan (Absolute)", "Tilt (Absolute)" };

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

  for (auto const& control : d_controls)
  {
    string name = control->name;
    V4L2ControlType type = control->type;
    const bool isReadOnly = false;

    // Remove some we cannot use
    if (ignoreControlNames.find(name) != ignoreControlNames.end())
      continue;

    bool isAdvanced = advancedControlNames.find(name) != advancedControlNames.end();

    // Shorten some verbose names
    if (name == "White Balance Temperature, Auto")
      name = "Auto WB";
    else if (name == "White Balance Temperature")
      name = "WB Temp (K)";

    // Reinterpret some two-valued INT controls as BOOL
    if (type == V4L2ControlType::CT_INT && control->minimum == 0 && control->maximum == 1)
      type = V4L2ControlType::CT_BOOL;

    // Create a path from this control's name. This will be used in the config file.
    stringstream path;
    path << "camera.settings.";
    for (char const& c : name)
    {
      if (c == ' ')
        path << '-';
      else
        path << tolower(c);
    }

    int currentValue = getValue(control->id);

    // Create the appropriate type of Setting<T>
    switch (type)
    {
      case V4L2ControlType::CT_BOOL:
      {
        auto setting = new BoolSetting(path.str(), control->defaultValue, isReadOnly, isAdvanced);
        setting->setValue(currentValue != 0);
        setting->changed.connect([setValue,control](bool value) { setValue(control->id, value ? 1 : 0); });
        d_settings.push_back(setting);
        break;
      }
      case V4L2ControlType::CT_INT:
      {
        auto setting = new IntSetting(path.str(), control->minimum, control->maximum, control->defaultValue, isReadOnly, isAdvanced);
        setting->setValue(currentValue);
        setting->changed.connect([setValue,control](int value) { setValue(control->id, value); });
        d_settings.push_back(setting);
        break;
      }
      case V4L2ControlType::CT_MENU:
      {
        struct v4l2_querymenu querymenu;
        memset (&querymenu, 0, sizeof(querymenu));
        querymenu.id = control->id;

        // Query all enum values
        map<int,string> pairs;
        for (unsigned i = control->minimum; i <= control->maximum; i++)
        {
          querymenu.index = i;
          if (ioctl(d_fd, VIDIOC_QUERYMENU, &querymenu) == 0)
            pairs[i] = (const char*)querymenu.name;
        }

        auto setting = new EnumSetting(path.str(), pairs, control->defaultValue, isReadOnly, isAdvanced);
        setting->setValue(currentValue);
        setting->changed.connect([setValue,control](int value) { setValue(control->id, value); });
        d_settings.push_back(setting);
        break;
      }
      default:
      {
        cerr << "[Camera::createControls] Unsupported camera control type: " << (int)type << endl;
      }
    }
  }
}
