#include "actionoption.ih"

std::vector<std::shared_ptr<Option>> ActionOption::runPolicy()
{
  if (!d_runner || d_runner->getState() != MotionScriptRunnerState::Running)
  {
    bool started = d_actionModule->start(d_runner);

    if (started)
      cout << "[ActionOption::runPolicy] Started action: " << getID() << endl;
    else
      cout << "[ActionOption::runPolicy] Request to start action denied: " << getID() << endl;
  }

  return std::vector<std::shared_ptr<Option>>();
}
