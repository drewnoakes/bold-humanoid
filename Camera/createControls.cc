#include "camera.ih"

#include <cctype>

using namespace rapidjson;

void Camera::createControls()
{
  d_controls.clear();

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

    map<int,string> pairs;
    if (queryctrl.type == (uint)V4L2ControlType::CT_MENU)
    {
      struct v4l2_querymenu querymenu;
      memset (&querymenu, 0, sizeof(querymenu));
      querymenu.id = queryctrl.id;

      // Query all enum values
      for (unsigned i = queryctrl.minimum; i <= queryctrl.maximum; i++)
      {
        querymenu.index = i;
        if (ioctl(d_fd, VIDIOC_QUERYMENU, &querymenu) == 0)
          pairs[i] = (const char*)querymenu.name;
      }
    }

    d_controls.push_back(make_shared<Control>(queryctrl, pairs));

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

  auto setValue = [this,getValue](shared_ptr<Control const> const& control, int const& value)
  {
    v4l2_control ctrl = {0,};
    ctrl.id = control->id;
    ctrl.value = value;
    ioctl(d_fd, VIDIOC_S_CTRL, &ctrl);

    // Test whether the value we set was taken or not
    int retrieved = getValue(control->id);

    if (retrieved != value)
      cerr << ccolor::error << "[Camera::setValue] Setting camera control '" << control->name << "' failed -- set " << value << " but read back " << retrieved << ccolor::reset << endl;
  };

  vector<SettingBase*> settings;

  for (shared_ptr<Control const> const& control : d_controls)
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
      if (c == '(' || c == ')' || c ==',')
        continue;
      else if (c == ' ')
        path << '-';
      else
        path << (char)tolower(c);
    }

    int currentValue = getValue(control->id);

    // Create the appropriate type of Setting<T>
    switch (type)
    {
      case V4L2ControlType::CT_BOOL:
      {
        auto setting = new BoolSetting(path.str(), control->defaultValue, isReadOnly, isAdvanced, name);
        setting->setValue(currentValue != 0);
        setting->changed.connect([setValue,control](bool value) { setValue(control, value ? 1 : 0); });
        settings.push_back(setting);
        break;
      }
      case V4L2ControlType::CT_INT:
      {
        auto setting = new IntSetting(path.str(), control->minimum, control->maximum, control->defaultValue, isReadOnly, isAdvanced, name);
        setting->setValue(currentValue);
        setting->changed.connect([setValue,control](int value) { setValue(control, value); });
        settings.push_back(setting);
        break;
      }
      case V4L2ControlType::CT_MENU:
      {
        auto setting = new EnumSetting(path.str(), control->pairs, control->defaultValue, isReadOnly, isAdvanced, name);
        setting->setValue(currentValue);
        setting->changed.connect([setValue,control](int value) { setValue(control, value); });
        settings.push_back(setting);
        break;
      }
      default:
      {
        cerr << ccolor::error << "[Camera::createControls] Unsupported camera control type: " << (int)type << ccolor::reset << endl;
      }
    }
  }

  // Camera settings must be set in a particular order. For example, you cannot
  // explicitly set the white balance when 'auto wb' mode is on. The value will
  // not stick.
  //
  // We use the order in which they're specified in the config document to
  // signify the order they should be set in.
  //
  // Finally, any setting which were not referenced in the config file will be
  // logged out, and appended in whatever order they were reported by the camera
  // in.

  Value const* jsonValue = Config::getConfigJsonValue("camera.settings");

  for (Value::Member const* member = jsonValue->MemberBegin(); member != jsonValue->MemberEnd(); ++member)
  {
    string name = member->name.GetString();
    stringstream pathstream;
    pathstream << "camera.settings" << '.' << name;
    string path = pathstream.str();

    auto match = std::find_if(settings.begin(), settings.end(), [path](SettingBase* setting) { return setting->getPath() == path; });

    if (match == settings.end())
    {
      cerr << ccolor::warning << "[Camera::createControls] Configuration document specifies '" << path << "' but no setting exists with that name" << ccolor::reset << endl;
      continue;
    }

    SettingBase* setting = *match;

    // This is not very efficient (O(N)) but this only occurs at startup, so...
    settings.erase(match);

    Config::addSetting(setting);
  }

  for (SettingBase* leftOver : settings)
  {
    cerr << ccolor::warning << "[Camera::createControls] Configuration document doesn't specify a value for camera setting: " << leftOver->getPath() << ccolor::reset << endl;
    Config::addSetting(leftOver);
  }
}
