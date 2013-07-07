#include "actionoption.ih"

std::vector<std::shared_ptr<Option>> ActionOption::runPolicy()
{
  assert(d_actionPage != ActionPage::None || d_actionName != "");

  // TODO shouldn't have to check if something's already running -- can just request it and the system should sort it out
  if (!d_started && !d_actionModule->isRunning())
  {
    cout << "[ActionOption::runPolicy] Starting action: " << getID() << endl;

    if (d_actionPage != ActionPage::None)
      d_actionModule->start((int)d_actionPage);
    else
      d_actionModule->start(d_actionName);

    d_started = true;
  }

  return std::vector<std::shared_ptr<Option>>();
}
