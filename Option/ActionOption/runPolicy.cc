#include "actionoption.ih"

std::vector<std::shared_ptr<Option>> ActionOption::runPolicy()
{
  assert(d_actionPage != ActionPage::None || d_actionName != "");
      
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
