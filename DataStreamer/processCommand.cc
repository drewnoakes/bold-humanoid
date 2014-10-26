#include "datastreamer.ih"

void DataStreamer::processCommand(string json, JsonSession* jsonSession)
{
  log::info("DataStreamer::processCommand") << "Processing: " << json;

  Document doc;
  doc.Parse<0>(json.c_str());

  if (doc.HasParseError())
  {
    log::error("DataStreamer::processCommand") << "Error parsing command JSON";
    return;
  }

  char const* type;
  if (!doc.TryGetStringValue("type", &type))
  {
    log::error("DataStreamer::processCommand") << "No 'type' specified in received command JSON";
    return;
  }

  if (strcmp(type, "action") == 0)
  {
    //
    // Handle ACTION
    //

    // { "type": "action", "id": "some.action" }
    // { "type": "action", "id": "some.action", "args": ... }

    char const* id;
    if (!doc.TryGetStringValue("id", &id))
    {
      log::error("DataStreamer::processCommand") << "No 'id' specified in received action JSON";
      return;
    }

    auto action = Config::getAction(string(id));

    if (!action)
    {
      log::error("DataStreamer::processCommand") << "No action exists with id: " << id;
      return;
    }

    Value::Member* argsMember = doc.FindMember("args");
    action->handleRequest(argsMember ? &argsMember->value : nullptr);
  }
  else if (strcmp(type, "setting") == 0)
  {
    //
    // Handle SETTING
    //

    // { "type": "setting", "path": "some.setting", "value": 1234 }

    char const* path;
    if (!doc.TryGetStringValue("path", &path))
    {
      log::error("DataStreamer::processCommand") << "No 'path' specified in received setting JSON";
      return;
    }

    auto valueMember = doc.FindMember("value");
    if (!valueMember)
    {
      log::error("DataStreamer::processCommand") << "No 'value' specified in received setting JSON";
      return;
    }

    auto setting = Config::getSettingBase(path);

    if (!setting)
    {
      log::error("DataStreamer::processCommand") << "No setting exists with path: " << path;
      return;
    }

    if (!setting->setValueFromJson(&valueMember->value))
    {
      // Setting the value failed. Send the current value back to the client
      // that requested this invalid value.
      StringBuffer buffer;
      Writer<StringBuffer> writer(buffer);
      writeSettingUpdateJson(setting, writer);
      jsonSession->enqueue(buffer);
    }
  }
}
