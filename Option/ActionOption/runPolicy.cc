#include "actionoption.ih"

OptionList ActionOption::runPolicy()
{
  assert(d_actionPage != ActionPage::None || d_actionName != "");
      
  if (!d_started && !d_actionModule->isRunning())
  {
    cout << "[ActionOption::runPolicy] Starting action: " << getID() << endl;

//     d_actionModule->d_jointData.setEnableBody(true, true);
    if (d_actionPage != ActionPage::None)
      d_actionModule->start(d_actionPage);
    else
      d_actionModule->start(d_actionName);

    d_started = true;
  }

  return OptionList();
}
