#include "optiontreestate.ih"

void OptionTreeState::writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("ranoptions");
    writer.StartArray();
    for (auto const& option : d_ranOptions)
      writer.String(option->getId().c_str());
    writer.EndArray();

    writer.String("fsms");
    writer.StartArray();
    for (auto const& fsmState : d_fsmStates)
    {
      writer.StartObject();
      writer.String("fsm");
      writer.String(fsmState.getFsmName().c_str());
      writer.String("state");
      writer.String(fsmState.getStateName().c_str());
      writer.EndObject();
    }
    writer.EndArray();

    // Inject the option JSON document at this point in our output
    writer.String("path");
    d_optionJson->Accept(writer);
  }
  writer.EndObject();
}
