#include "datastreamer.ih"

void DataStreamer::sendCameraControls(libwebsocket* wsi)
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

  writeJson(wsi, buffer);
}
