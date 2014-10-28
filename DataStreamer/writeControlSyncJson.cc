#include "datastreamer.ih"

#include "../OptionTree/optiontree.hh"
#include "../Option/FSMOption/fsmoption.hh"
#include "../MotionScript/motionscript.hh"

void DataStreamer::writeControlSyncJson(Writer<StringBuffer>& writer)
{
  writer.StartObject();
  {
    writer.String("files");
    writer.StartArray();
    {
      for (auto const& fileName : Config::getConfigDocumentNames())
        writer.String(fileName.c_str());
    }
    writer.EndArray();

    writer.String("type");
    writer.String("sync");

    writer.String("actions");
    writer.StartArray();
    {
      for (Action const* action : Config::getAllActions())
      {
        writer.StartObject();
        {
          writer.String("id");
          writer.String(action->getId().c_str());

          writer.String("label");
          writer.String(action->getLabel().c_str());

          writer.String("hasArguments");
          writer.Bool(action->hasArguments());
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
        fsm->toJson(writer);
    }
    writer.EndArray();

    writer.String("motionScripts");
    writer.StartArray();
    {
      for (auto const& motionScript : MotionScript::loadAllInPath("./motionscripts"))
        motionScript->writeJson(writer);
    }
    writer.EndArray();
  }
  writer.EndObject();
}
