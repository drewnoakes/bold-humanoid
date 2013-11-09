#include "motionscriptoption.ih"

vector<shared_ptr<Option>> MotionScriptOption::runPolicy()
{
  cout << "[MotionScriptOption::runPolicy] " << getID() << endl;

  if (!d_runner || d_runner->getState() != MotionScriptRunnerState::Running)
  {
    auto runner = make_shared<MotionScriptRunner>(d_script);

    bool started = d_motionScriptModule->start(runner);

    if (started)
    {
      cout << "[MotionScriptOption::runPolicy] Started motion script: " << getID() << endl;
      d_runner = runner;
    }
    else
    {
      cout << "[MotionScriptOption::runPolicy] Request to start motion script denied: " << getID() << endl;
    }
  }

  return vector<shared_ptr<Option>>();
}
