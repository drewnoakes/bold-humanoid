#include "actionmodule.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../MotionScriptRunner/motionscriptrunner.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../ThreadId/threadid.hh"

#include <cassert>
#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

ActionModule::ActionModule(shared_ptr<MotionTaskScheduler> scheduler, vector<shared_ptr<MotionScript>> scripts)
: MotionModule("action", scheduler)
{
  // Sort scripts alphabetically by name
  sort(scripts.begin(), scripts.end(),
      [](shared_ptr<MotionScript> const& a, shared_ptr<MotionScript> const& b) -> bool
  {
    return a->getName() < b->getName();
  });

  for (shared_ptr<MotionScript> script : scripts)
  {
    cout << "[ActionModule::ActionModule] Found script: " << script->getName() << endl;
    d_controls.push_back(Control::createAction(script->getName(), [this,script]() { start(make_shared<MotionScriptRunner>(script)); }));
  }
}

void ActionModule::initialize()
{
  // TODO is this necessary?
  d_runner = nullptr;
}

bool ActionModule::isRunning()
{
  return d_runner && d_runner->getState() != MotionScriptRunnerState::Finished;
}

void ActionModule::step(shared_ptr<JointSelection> selectedJoints)
{
  assert(ThreadId::isMotionLoopThread());

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

void ActionModule::applySection(shared_ptr<BodySection> section)
{
  if (!d_runner)
    return;

  section->visitJoints([&section,this](shared_ptr<JointControl> joint)
  {
    joint->setValue(d_runner->getValue(joint->getId()));
    joint->setPGain(d_runner->getPGain(joint->getId()));
  });
}

void ActionModule::applyHead(shared_ptr<HeadSection> head) { applySection(dynamic_pointer_cast<BodySection>(head)); }
void ActionModule::applyArms(shared_ptr<ArmSection> arms) { applySection(dynamic_pointer_cast<BodySection>(arms)); }
void ActionModule::applyLegs(shared_ptr<LegSection> legs) { applySection(dynamic_pointer_cast<BodySection>(legs)); }

bool ActionModule::start(shared_ptr<MotionScriptRunner> scriptRunner)
{
  cout << "[ActionModule::start] Starting script " << scriptRunner->getScriptName() << endl;

  if (d_runner && d_runner->getState() != MotionScriptRunnerState::Finished)
  {
    cerr << "[ActionModule::start] Ignoring request to play script " << scriptRunner->getScriptName()
         << " -- already playing " << d_runner->getScriptName()
         << endl;

    return false;
  }

  d_runner = scriptRunner;

  // NOTE currently we assume that motion scripts control all body parts

  getScheduler()->add(this,
                      Priority::Optional,  true,  // HEAD   Interuptable::YES
                      Priority::Important, true,  // ARMS
                      Priority::Important, true); // LEGS

  return true;
}
