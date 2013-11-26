#include "datastreamer.ih"

shared_ptr<vector<uchar>> DataStreamer::prepareControlSyncBytes()
{
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  writer.StartObject();
  {
    writer.String("type").String("sync");

    writer.String("actions");
    writer.StartArray();
    {
      for (Action const* action : Config::getAllActions())
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
      for (SettingBase const* setting : Config::getAllSettings())
      {
        setting->writeFullJson(writer);
      }
    }
    writer.EndArray();
  }
  writer.EndObject();

  return JsonSession::createBytes(buffer);
}
