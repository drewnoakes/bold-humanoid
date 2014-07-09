#include "datastreamer.ih"

shared_ptr<vector<uchar>> DataStreamer::prepareSettingUpdateBytes(SettingBase const* setting)
{
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  writer.StartObject();
  {
    writer.String("type").String("update");
    writer.String("path").String(setting->getPath().c_str());
    writer.String("value");
    setting->writeJsonValue(writer);
  }
  writer.EndObject();

  return JsonSession::createBytes(buffer);
}
