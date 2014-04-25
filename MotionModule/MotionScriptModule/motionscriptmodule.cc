#include "motionscriptmodule.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../Config/config.hh"
#include "../../MotionScriptRunner/motionscriptrunner.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../ThreadUtil/threadutil.hh"
#include "../../util/assert.hh"

using namespace bold;
using namespace std;

MotionScriptModule::MotionScriptModule(shared_ptr<MotionTaskScheduler> scheduler, vector<shared_ptr<MotionScript>> scripts)
: MotionModule("motion-script", scheduler)
{
  // Sort scripts alphabetically by name
  sort(scripts.begin(), scripts.end(),
      [](shared_ptr<MotionScript> const& a, shared_ptr<MotionScript> const& b) -> bool
  {
    return a->getName() < b->getName();
  });

  for (shared_ptr<MotionScript> script : scripts)
  {
    stringstream id;
    id << "motion-script." << script->getName();
    Config::addAction(id.str(), script->getName(), [this,script]() { start(make_shared<MotionScriptRunner>(script)); });
  }
  log::info("MotionScriptModule::MotionScriptModule") << "Loaded " << scripts.size() << " motion scripts";
}

bool MotionScriptModule::isRunning()
{
  return d_runner && d_runner->getState() != MotionScriptRunnerState::Finished;
}

void MotionScriptModule::step(shared_ptr<JointSelection> const& selectedJoints)
{
  ASSERT(ThreadUtil::isMotionLoopThread());

  if (!d_runner)
    return;

  if (d_runner->getState() == MotionScriptRunnerState::Finished)
  {
    d_runner = nullptr;
    return;
  }

  // TODO clarify what runner completion means: should we apply the section or not?

  if (!d_runner->step(selectedJoints))
    setCompletedFlag();
}

void MotionScriptModule::applyHead(HeadSection* head) { if (d_runner) d_runner->applyHead(head); }
void MotionScriptModule::applyArms(ArmSection*  arms) { if (d_runner) d_runner->applyArms(arms); }
void MotionScriptModule::applyLegs(LegSection*  legs) { if (d_runner) d_runner->applyLegs(legs); }

bool MotionScriptModule::start(shared_ptr<MotionScriptRunner> const& scriptRunner)
{
  auto const& script = scriptRunner->getScript();

  log::info("MotionScriptModule::start") << "Starting script " << script->getName();

  if (d_runner && d_runner->getState() != MotionScriptRunnerState::Finished)
  {
    log::warning("MotionScriptModule::start") << "Ignoring request to play script " << script->getName()
        << " -- already playing " << d_runner->getScript()->getName()
        << " in state " << MotionScriptRunner::getStateName(d_runner->getState());
    return false;
  }

  d_runner = scriptRunner;

  // NOTE currently we assume that motion scripts control all body parts

  getScheduler()->add(this,
                      Priority::Low,  scriptRunner->getScript()->getControlsHead(),  // HEAD   Interuptable::YES
                      Priority::High, scriptRunner->getScript()->getControlsArms(),  // ARMS
                      Priority::High, scriptRunner->getScript()->getControlsLegs()); // LEGS

  return true;
}
