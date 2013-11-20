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
      for (auto& pair1 : d_controlsByIdByFamily)
      {
        string family = pair1.first;

        writer.String(family.c_str());
        writer.StartArray();

        for (auto pair2 : pair1.second)
        {
          writer.StartObject();
          {
            auto action = pair2.second;
            writer.String("name").String(action->getName().c_str());
            writer.String("id").Uint(action->getId());
          }
          writer.EndObject();
        }

        writer.EndArray();
      }
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
