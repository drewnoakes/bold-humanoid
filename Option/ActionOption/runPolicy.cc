#include "actionoption.ih"

OptionList ActionOption::runPolicy()
{
  assert(d_actionPage != ActionPage::None || d_actionName != "");

  auto actionModule = robotis::Action::GetInstance();
  if (!d_started && !actionModule->IsRunning())
  {
    cout << "[ActionOption::runPolicy] Starting Action: " << getID() << endl;
    actionModule->m_Joint.SetEnableBody(true, true);
    if (d_actionPage != ActionPage::None)
      actionModule->Start((int)d_actionPage);
    else
      actionModule->Start(d_actionName.c_str());
      
    d_started = true;
  }
  return OptionList();
}
