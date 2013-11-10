#include "motionscriptoption.hh"

#include "../../MotionModule/MotionScriptModule/motionscriptmodule.hh"
#include "../../MotionScriptRunner/motionscriptrunner.hh"
#include "../../MotionScript/motionscript.hh"

#include <iostream>
#include <cassert>

using namespace bold;
using namespace std;

MotionScriptOption::MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::string const& fileName)
: Option(id),
  d_motionScriptModule(motionScriptModule),
  d_runner()
{
  d_script = MotionScript::fromFile(fileName);

  if (!d_script)
    throw runtime_error("File not found");
}

MotionScriptOption::MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::shared_ptr<MotionScript const> script)
: Option(id),
  d_motionScriptModule(motionScriptModule),
  d_script(script),
  d_runner()
{
  assert(d_script);
  assert(d_motionScriptModule);
}

double MotionScriptOption::hasTerminated()
{
  // If we haven't run yet, then we haven't terminated
  if (!d_runner)
    return 0;

  // TODO we are using the 'hasTerminated' function here to clear internal state, meaning the answer is not idempotent

  if (d_runner->getState() == MotionScriptRunnerState::Finished)
  {
//     cout << "[MotionScriptOption::hasTerminated] Motion script completed: " << getID() << endl;
    d_runner = nullptr;
    return 1.0;
  }

  return 0.0;
}

vector<shared_ptr<Option>> MotionScriptOption::runPolicy()
{
  if (!d_runner || d_runner->getState() == MotionScriptRunnerState::Finished)
  {
    auto runner = make_shared<MotionScriptRunner>(d_script);

    bool started = d_motionScriptModule->start(runner);

    if (started)
    {
//       cout << "[MotionScriptOption::runPolicy] Started motion script: " << getID() << endl;
      d_runner = runner;
    }
    else
    {
//       cout << "[MotionScriptOption::runPolicy] Request to start motion script denied: " << getID() << endl;
      d_runner = nullptr;
    }
  }

  return vector<shared_ptr<Option>>();
}
