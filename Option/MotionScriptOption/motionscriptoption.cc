#include "motionscriptoption.hh"

#include "../../MotionModule/MotionScriptModule/motionscriptmodule.hh"
#include "../../MotionScriptRunner/motionscriptrunner.hh"
#include "../../MotionScript/motionscript.hh"

#include <cassert>

using namespace bold;
using namespace rapidjson;
using namespace std;

MotionScriptOption::MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::string const& fileName)
: Option(id, "MotionScript"),
  d_motionScriptModule(motionScriptModule),
  d_runner()
{
  d_script = MotionScript::fromFile(fileName);

  if (!d_script)
    throw runtime_error("File not found");
}

MotionScriptOption::MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::shared_ptr<MotionScript const> script)
: Option(id, "MotionScript"),
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
    log::verbose("MotionScriptOption::hasTerminated") << "Motion script completed: " << getId();
    d_runner = nullptr;
    return 1.0;
  }

  return 0.0;
}

vector<shared_ptr<Option>> MotionScriptOption::runPolicy(Writer<StringBuffer>& writer)
{
  if (!d_runner || d_runner->getState() == MotionScriptRunnerState::Finished)
  {
    auto runner = make_shared<MotionScriptRunner>(d_script);

    bool started = d_motionScriptModule->start(runner);

    if (started)
    {
      log::verbose("MotionScriptOption::runPolicy") << "Started motion script: " << getId();
      d_runner = runner;
    }
    else
    {
      log::verbose("MotionScriptOption::runPolicy") << "Request to start motion script denied: " << getId();
      d_runner = nullptr;
    }

    writer.String("state").String(MotionScriptRunner::getStateName(d_runner->getState()).c_str());
  }

  return {};
}
