#include "datastreamer.ih"

#include "../OptionTree/optiontree.hh"
#include "../Option/FSMOption/fsmoption.hh"

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
          writer.String("hasArguments").Bool(action->hasArguments());
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

    writer.String("fsms");
    writer.StartArray();
    {
      for (auto const& fsm : d_optionTree->getFSMs())
      {
        fsm->toJson(writer);
      }
    }
    writer.EndArray();
  }
  writer.EndObject();

  return JsonSession::createBytes(buffer);
}
