#include "datastreamer.ih"

#include <rapidjson/error/en.h>

void DataStreamer::processCommand(string json, JsonSession* jsonSession)
{
  log::info("DataStreamer::processCommand") << "Processing: " << json;

  Document doc;
  doc.Parse<0>(json.c_str());

  if (doc.HasParseError())
  {
    log::error("DataStreamer::processCommand") << "Error parsing command JSON at offset " << doc.GetErrorOffset() << ": " << GetParseError_En(doc.GetParseError());
    return;
  }

  auto typeMember = doc.FindMember("type");
  if (typeMember == doc.MemberEnd() || !typeMember->value.IsString())
  {
    log::error("DataStreamer::processCommand") << "No 'type' specified in received command JSON";
    return;
  }
  char const* type = typeMember->value.GetString();

  if (strcmp(type, "action") == 0)
  {
    //
    // Handle ACTION
    //

    // { "type": "action", "id": "some.action" }
    // { "type": "action", "id": "some.action", "args": ... }

    auto idMember = doc.FindMember("id");
    if (idMember == doc.MemberEnd() || !idMember->value.IsString())
    {
      log::error("DataStreamer::processCommand") << "No 'id' specified in received action JSON";
      return;
    }

    char const* id = idMember->value.GetString();

    auto action = Config::getAction(string(id));

    if (!action)
    {
      log::error("DataStreamer::processCommand") << "No action exists with id: " << id;
      return;
    }

    auto argsMember = doc.FindMember("args");
    action->handleRequest(argsMember != doc.MemberEnd() ? &argsMember->value : nullptr);
  }
  else if (strcmp(type, "setting") == 0)
  {
    //
    // Handle SETTING
    //

    // { "type": "setting", "path": "some.setting", "value": 1234 }

    auto pathMember = doc.FindMember("path");
    if (pathMember == doc.MemberEnd() || !pathMember->value.IsString())
    {
      log::error("DataStreamer::processCommand") << "No 'path' specified in received setting JSON";
      return;
    }
    char const* path = pathMember->value.GetString();

    auto valueMember = doc.FindMember("value");
    if (valueMember == doc.MemberEnd())
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
      WebSocketBuffer buffer;
      Writer<WebSocketBuffer> writer(buffer);
      writeSettingUpdateJson(setting, writer);
      jsonSession->enqueue(move(buffer));
    }
  }
}
