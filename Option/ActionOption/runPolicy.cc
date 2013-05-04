#include "actionoption.ih"

OptionList ActionOption::runPolicy()
{
  auto actionModule = robotis::Action::GetInstance();
  if (!d_started && !actionModule->IsRunning())
  {
    cout << "[ActionOption::runPolicy] Starting Action: " << getID() << endl;
    actionModule->m_Joint.SetEnableBody(true, true);
    actionModule->Start(d_actionName.c_str());
    d_started = true;
  }
  return OptionList();
}
