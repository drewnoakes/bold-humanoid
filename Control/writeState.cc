#include "control.ih"

void Control::writeState(Writer<StringBuffer>& writer) const
{
  // TODO SETTINGS doesn't do anything that the caller couldn't do

  writer.String("name");
  writer.String(d_name.c_str());

  writer.String("id");
  writer.Uint(d_id);
}
