#include "optiontreestate.ih"

void OptionTreeState::writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("ranoptions");
    writer.Array(
      d_ranOptions.begin(), d_ranOptions.end(),
      [&](shared_ptr<Option> const& option) { writer.String(option->getId().c_str()); });

    writer.String("fsms");
    writer.Array(
      d_fsmStates.begin(), d_fsmStates.end(),
      [&](FSMStateSnapshot const& fsmState) {
        writer.StartObject()
          .String("fsm").String(fsmState.getFsmName().c_str())
          .String("state").String(fsmState.getStateName().c_str())
          .EndObject();
      });

    // Inject the option JSON document at this point in our output
    writer.String("path");
    d_optionJson->Accept(writer);
  }
  writer.EndObject();
}
