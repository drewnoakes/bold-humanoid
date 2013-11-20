#include "control.ih"

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
  control->d_name = name;
  control->d_callback = callback;
  return control;
}
