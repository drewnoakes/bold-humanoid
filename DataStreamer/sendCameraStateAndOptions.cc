#include "datastreamer.ih"

void DataStreamer::sendCameraStateAndOptions(libwebsocket* wsi)
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

      for (auto& pair2 : pair1.second)
      {
        writer.StartObject();
        Control control = pair2.second;
        control.writeState(writer);
        writer.EndObject();
      }

      writer.EndArray();
    }
  }
  writer.EndObject();

  const char* json = buffer.GetString();

  cout << "[DataStreamer::sendCameraStateAndOptions] sending: " << json << endl;

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING +
                    buffer.GetSize() +
                    LWS_SEND_BUFFER_POST_PADDING];
  unsigned char* p = buf + LWS_SEND_BUFFER_POST_PADDING;
  memcpy(p, buffer.GetString(), buffer.GetSize());

  int res = libwebsocket_write(wsi, p, buffer.GetSize(), LWS_WRITE_TEXT);
}
