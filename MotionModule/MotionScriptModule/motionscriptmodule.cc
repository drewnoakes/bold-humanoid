#include "motionscriptmodule.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../Config/config.hh"
#include "../../MotionScriptRunner/motionscriptrunner.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../ThreadUtil/threadutil.hh"
#include "../../util/assert.hh"

#include <rapidjson/document.h>

using namespace bold;
using namespace rapidjson;
using namespace std;

void MotionScriptModule::createActions(string const& path, shared_ptr<MotionScriptModule> const& module)
{
  vector<shared_ptr<MotionScript>> scripts = MotionScript::loadAllInPath(path);

  // Sort scripts alphabetically by name
  sort(scripts.begin(), scripts.end(),
      [](shared_ptr<MotionScript> const& a, shared_ptr<MotionScript> const& b) -> bool
  {
    return a->getName() < b->getName();
  });

  // Add an action for each known motion script
  for (shared_ptr<MotionScript> script : scripts)
  {
    stringstream id;
    id << "motion-script." << script->getName();
    Config::addAction(id.str(), script->getName(), [module,script] { module->run(script); });
  }

  Config::addAction("motion-module.play-script-content", "Play Script", [module](Value* value)
  {
    auto script = MotionScript::fromJsonValue(*value);
    module->run(script);
  });

  log::info("MotionScriptModule::MotionScriptModule") << "Loaded " << scripts.size() << " motion scripts";
}

//////////////////////////////////////////////////////////////////////////////

MotionScriptModule::MotionScriptModule(shared_ptr<MotionTaskScheduler> scheduler)
: MotionModule("motion-script", scheduler)
{}

bool MotionScriptModule::isRunning() const
{
  return d_runner && d_runner->getStatus() != MotionScriptRunnerStatus::Finished
      && d_motionRequest && !d_motionRequest->hasCompleted();
}

void MotionScriptModule::step(shared_ptr<JointSelection> const& selectedJoints)
{
  ASSERT(ThreadUtil::isMotionLoopThread());

  if (!d_runner)
    return;

  if (d_runner->getStatus() == MotionScriptRunnerStatus::Finished)
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

shared_ptr<MotionRequest const> MotionScriptModule::run(shared_ptr<MotionScript const> const& script)
{
  return run(make_shared<MotionScriptRunner>(script));
}

shared_ptr<MotionRequest const> MotionScriptModule::run(shared_ptr<MotionScriptRunner> const& scriptRunner)
{
  // TODO thread safety!

  ASSERT(ThreadUtil::isThinkLoopThread(false) || ThreadUtil::isDataStreamerThread());

  auto const& script = scriptRunner->getScript();

  log::verbose("MotionScriptModule::run") << "Request to run script: " << script->getName();

  if (isRunning())
  {
    log::warning("MotionScriptModule::run") << "Ignoring request to play script " << script->getName()
        << " -- already playing " << d_runner->getScript()->getName()
        << " with status " << getMotionScriptRunnerStatusName(d_runner->getStatus());

    // An empty MotionRequest has Ignored status
    return make_shared<MotionRequest const>();
  }

  d_runner = scriptRunner;

  // NOTE currently we assume that motion scripts control all body parts

  d_motionRequest = getScheduler()->request(
    this,
    Priority::Low,  Required::No,  scriptRunner->getScript()->getControlsHead() ? RequestCommit::Yes : RequestCommit::No,  // HEAD
    Priority::High, Required::Yes, scriptRunner->getScript()->getControlsArms() ? RequestCommit::Yes : RequestCommit::No,  // ARMS
    Priority::High, Required::Yes, scriptRunner->getScript()->getControlsLegs() ? RequestCommit::Yes : RequestCommit::No); // LEGS

  return d_motionRequest;
}
