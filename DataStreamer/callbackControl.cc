#include "datastreamer.ih"

int DataStreamer::callback_control(
  struct libwebsocket_context* context,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*session*/,
  void* in,
  size_t len)
{
  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    libwebsocket_callback_on_writable(context, wsi);
    return 0;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject();
    {
      writer.String("actions");
      writer.StartArray();
      {
        for (Action* action : Config::getAllActions())
        {
          writer.StartObject();
          {
            writer.String("id").String(action->getId().c_str());
            writer.String("label").String(action->getLabel().c_str());
          }
          writer.EndObject();
        }
      }
      writer.EndArray();

      writer.String("settings");
      writer.StartArray();
      {
        for (SettingBase* setting : Config::getAllSettings())
        {
          writer.StartObject();
          {
            // TODO SETTINGS write out details of all Settings objects as well
          }
          writer.EndObject();
        }
      }
      writer.EndArray();
    }
    writer.EndObject();

    writeJson(wsi, buffer);
    break;
  }
  case LWS_CALLBACK_RECEIVE:
  {
    if (len != 0)
    {
      string str((char const*)in, len);
      processCommand(str);
    }
    break;
  }
  default:
    // Unknown reason
    break;
  }

  return 0;
}
