#include "actionoption.ih"

OptionList ActionOption::runPolicy()
{
  auto actionModule = Robot::Action::GetInstance();
  if (!d_started && !actionModule->IsRunning())
  {
    cout << "starting stand up" << endl;
    actionModule->m_Joint.SetEnableBody(true, true);
    actionModule->Start(d_actionName.c_str());
    d_started = true;
  }
  return OptionList();
}
