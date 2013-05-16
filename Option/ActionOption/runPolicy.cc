#include "actionoption.ih"

OptionList ActionOption::runPolicy()
{
  if (!d_started && !d_actionModule->isRunning())
  {
    cout << "[ActionOption::runPolicy] Starting action: " << getID() << endl;

//     d_actionModule->d_jointData.setEnableBody(true, true);
    d_actionModule->start(d_actionName);
    d_started = true;
  }

  return OptionList();
}
