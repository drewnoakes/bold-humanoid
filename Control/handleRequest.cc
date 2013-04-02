#include "control.ih"

bool Control::handleRequest(Document const& json)
{
  switch (d_type)
  {
  case ControlType::Int:
  case ControlType::Enum:
  {
    int value;
    if (!json.TryGetIntValue("value", &value))
    {
      cerr << "[Control::handleRequest] Int/enum type control request must specify an int 'value'." << endl;
      return false;
    }
    setValue(value);
    break;
  }
  
  case ControlType::Bool:
  {
    bool value;
    if (!json.TryGetBoolValue("value", &value))
    {
      cerr << "[Control::handleRequest] Bool type control request must specify a bool 'value'." << endl;
      return false;
    }
    setValue(value ? 1 : 0);
    break;
  }
  
  case ControlType::Action:
  {
    d_callback(0);
    break;
  }
  
  case ControlType::Unknown:
    break;
  }
  
  return true;
}
