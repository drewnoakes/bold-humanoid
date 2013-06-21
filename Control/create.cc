#include "control.ih"

shared_ptr<Control> Control::createInt(string name,
                                       std::function<int()> getter,
                                       function<void(int const& value)> callback)
{
  return createInt(s_nextControlId++, name, getter, callback);
}

shared_ptr<Control> Control::createInt(unsigned id,
                                       string name,
                                       std::function<int()> getter,
                                       function<void(int const& value)> callback)
{
  auto control = make_shared<Control>();
  control->d_id = id;
  control->d_type = ControlType::Int;
  control->d_name = name;
  control->d_getter = getter;
  control->d_setter = callback;
  return control;
}

shared_ptr<Control> Control::createBool(string name,
                                        std::function<bool()> getter,
                                        function<void(bool const& value)> callback)
{
  return createBool(s_nextControlId++, name, getter, callback);
}

shared_ptr<Control> Control::createBool(unsigned id,
                                        string name,
                                        std::function<bool()> getter,
                                        function<void(bool const& value)> callback)
{
  auto control = make_shared<Control>();
  control->d_id = id;
  control->d_type = ControlType::Bool;
  control->d_name = name;
  control->d_getter = [getter]() { return getter() ? 1 : 0; };
  control->d_setter = [callback](int const& value) { callback(value != 0); };
  return control;
}

shared_ptr<Control> Control::createEnum(string name,
                                        vector<ControlEnumValue> enumValues,
                                        std::function<unsigned()> getter,
                                        function<void(ControlEnumValue const& value)> callback)
{
  return createEnum(s_nextControlId++, name, enumValues, getter, callback);
}

shared_ptr<Control> Control::createEnum(unsigned id,
                                        string name,
                                        vector<ControlEnumValue> enumValues,
                                        std::function<unsigned()> getter,
                                        function<void(ControlEnumValue const& value)> callback)
{
  auto control = make_shared<Control>();
  control->d_id = id;
  control->d_type = ControlType::Enum;
  control->d_name = name;
  control->d_getter = [getter]() { return (int)getter(); };
  control->d_setter = [callback,enumValues](int const& value)
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
  control->d_enumValues = enumValues;
  return control;
}

shared_ptr<Control> Control::createAction(string name,
                                          function<void()> callback)
{
  return createAction(s_nextControlId++, name, callback);
}

shared_ptr<Control> Control::createAction(unsigned id,
                                          string name,
                                          function<void()> callback)
{
  auto control = make_shared<Control>();
  control->d_id = id;
  control->d_type = ControlType::Action;
  control->d_name = name;
  control->d_setter = [callback](int const& value) { callback(); };
  return control;
}
