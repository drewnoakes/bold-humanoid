#include "actionoption.ih"

std::vector<std::shared_ptr<Option>> ActionOption::runPolicy()
{
  assert(d_actionPage != ActionPage::None || d_actionName != "");

  if (!d_runner || d_runner->getState() != MotionScriptRunnerState::Running)
  {
    if (d_actionPage != ActionPage::None)
      d_runner = d_actionModule->start((int)d_actionPage);
    else
      d_runner = d_actionModule->start(d_actionName);

    if (d_runner)
      cout << "[ActionOption::runPolicy] Started action: " << getID() << endl;
    else
      cout << "[ActionOption::runPolicy] Request to start action denied: " << getID() << endl;
  }

  return std::vector<std::shared_ptr<Option>>();
}
