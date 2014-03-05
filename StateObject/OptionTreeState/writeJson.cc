#include "optiontreestate.ih"

void OptionTreeState::writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("ranoptions");
    writer.StartArray();
    for (auto option : d_ranOptions)
      writer.String(option->getId().c_str());
    writer.EndArray();
  }
  writer.EndObject();
}
