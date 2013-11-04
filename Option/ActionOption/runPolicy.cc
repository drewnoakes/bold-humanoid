#include "actionoption.ih"

vector<shared_ptr<Option>> ActionOption::runPolicy()
{
  cout << "[ActionOption::runPolicy] " << getID() << endl;
  
  if (!d_runner || d_runner->getState() != MotionScriptRunnerState::Running)
  {
    auto runner = make_shared<MotionScriptRunner>(d_script);

    bool started = d_actionModule->start(runner);

    if (started)
    {
      cout << "[ActionOption::runPolicy] Started action: " << getID() << endl;
      d_runner = runner;
    }
    else
    {
      cout << "[ActionOption::runPolicy] Request to start action denied: " << getID() << endl;
    }
  }

  return vector<shared_ptr<Option>>();
}
