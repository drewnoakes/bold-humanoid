#include "datastreamer.ih"

void DataStreamer::writeSettingUpdateJson(SettingBase const* setting, Writer<StringBuffer>& writer)
{
  writer.StartObject();
  {
    writer.String("type");
    writer.String("update");

    writer.String("path");
    writer.String(setting->getPath().c_str());

    writer.String("value");
    setting->writeJsonValue(writer);
  }
  writer.EndObject();
}
