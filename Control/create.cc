#include "control.ih"

Control Control::createInt(string name,
                           int value,
                           function<void(int const& value)> callback)
{
  return createInt(s_nextControlId++, name, value, callback);
}

Control Control::createInt(unsigned id,
                           string name,
                           int value,
                           function<void(int const& value)> callback)
{
  auto control = Control();
  control.d_id = id;
  control.d_type = ControlType::Int;
  control.d_name = name;
  control.d_value = value;
  control.d_callback = callback;
  return control;
}

Control Control::createBool(string name,
                            bool value,
                            function<void(bool const& value)> callback)
{
  return createBool(s_nextControlId++, name, value, callback);
}

Control Control::createBool(unsigned id,
                            string name,
                            bool value,
                            function<void(bool const& value)> callback)
{
  auto control = Control();
  control.d_id = id;
  control.d_type = ControlType::Bool;
  control.d_name = name;
  control.d_value = value;
  control.d_callback = [callback](int const& value) { callback(value != 0); };
  return control;
}

Control Control::createEnum(string name,
                            vector<ControlEnumValue> enumValues,
                            unsigned value,
                            function<void(ControlEnumValue const& value)> callback)
{
  return createEnum(s_nextControlId++, name, enumValues, value, callback);
}

Control Control::createEnum(unsigned id,
                            string name,
                            vector<ControlEnumValue> enumValues,
                            unsigned value,
                            function<void(ControlEnumValue const& value)> callback)
{
  auto control = Control();
  control.d_id = id;
  control.d_type = ControlType::Enum;
  control.d_name = name;
  control.d_value = value;
  control.d_callback = [callback,enumValues](int const& value)
  {
    for (auto const& enumValue : enumValues)
    {
      if (enumValue.getValue() == value)
      {
        callback(enumValue);
        return;
      }
    }
    cerr << "Unable to find enum member with value " << value << endl;
  };
  control.d_enumValues = enumValues;
  return control;
}

Control Control::createAction(string name,
                              function<void()> callback)
{
  return createAction(s_nextControlId++, name, callback);
}

Control Control::createAction(unsigned id,
                              string name,
                              function<void()> callback)
{
  auto control = Control();
  control.d_id = id;
  control.d_type = ControlType::Action;
  control.d_name = name;
  control.d_callback = [callback](int const& value) { callback(); };
  return control;
}
