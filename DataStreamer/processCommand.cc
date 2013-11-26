#include "datastreamer.ih"

void DataStreamer::processCommand(string json, JsonSession* jsonSession, libwebsocket_context* context, libwebsocket* wsi)
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

    // { "type": "action", "id": "some.action" }

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

    // { "type": "setting", "path": "some.setting", "value": 1234 }

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

    if (!setting->setValueFromJson(&valueMember->value))
    {
      // Setting the value failed. Send the current value back to the client
      // that requested this invalid value.
      jsonSession->queue.push(prepareSettingUpdateBytes(setting));
      libwebsocket_callback_on_writable(context, wsi);
    }
  }
}
