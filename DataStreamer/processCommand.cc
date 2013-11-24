#include "datastreamer.ih"

void DataStreamer::processCommand(std::string json)
{
  cout << "[DataStreamer::processCommand] Processing: " << json << endl;

  rapidjson::Document d;

  d.Parse<0>(json.c_str());

  if (d.HasParseError())
  {
    cerr << ccolor::error << "[DataStreamer::processCommand] Error parsing command JSON" << ccolor::reset << endl;
    return;
  }

  char const* type;
  if (!d.TryGetStringValue("type", &type))
  {
    cerr << ccolor::error << "[DataStreamer::processCommand] No 'type' specified in received command JSON" << ccolor::reset << endl;
    return;
  }

  if (strcmp(type, "action") == 0)
  {
    //
    // Handle ACTION
    //

    char const* id;
    if (!d.TryGetStringValue("id", &id))
    {
      cerr << ccolor::error << "[DataStreamer::processCommand] No 'id' specified in received action JSON" << ccolor::reset << endl;
      return;
    }

    auto action = Config::getAction(string(id));

    if (!action)
    {
      cerr << ccolor::error << "[DataStreamer::processCommand] No action exists with id: " << id << ccolor::reset << endl;
      return;
    }

    action->handleRequest();
  }
  else if (strcmp(type, "setting") == 0)
  {
    //
    // Handle SETTING
    //

    char const* path;
    if (!d.TryGetStringValue("path", &path))
    {
      cerr << ccolor::error << "[DataStreamer::processCommand] No 'path' specified in received setting JSON" << ccolor::reset << endl;
      return;
    }

    auto valueMember = d.FindMember("value");
    if (!valueMember)
    {
      cerr << ccolor::error << "[DataStreamer::processCommand] No 'value' specified in received setting JSON" << ccolor::reset << endl;
      return;
    }

    auto setting = Config::getSettingBase(path);

    if (!setting)
    {
      cerr << ccolor::error << "[DataStreamer::processCommand] No setting exists with path: " << path << ccolor::reset << endl;
      return;
    }

    setting->setValueFromJson(&valueMember->value);
  }
}
