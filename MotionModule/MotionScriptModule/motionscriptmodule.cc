#include "motionscriptmodule.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../Config/config.hh"
#include "../../MotionScriptRunner/motionscriptrunner.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../ThreadId/threadid.hh"

#include <cassert>
#include <iostream>
#include <string.h>

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
  cout << "[MotionScriptModule::MotionScriptModule] Loaded " << scripts.size() << " motion scripts" << endl;
}

void MotionScriptModule::initialize()
{
  // TODO is this necessary?
  d_runner = nullptr;
}

bool MotionScriptModule::isRunning()
{
  return d_runner && d_runner->getState() != MotionScriptRunnerState::Finished;
}

void MotionScriptModule::step(shared_ptr<JointSelection> selectedJoints)
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

void MotionScriptModule::applySection(shared_ptr<BodySection> section)
{
  if (!d_runner)
    return;

  section->visitJoints([&section,this](shared_ptr<JointControl> joint)
  {
    joint->setValue(d_runner->getValue(joint->getId()));
    joint->setPGain(d_runner->getPGain(joint->getId()));
  });
}

void MotionScriptModule::applyHead(shared_ptr<HeadSection> head) { applySection(dynamic_pointer_cast<BodySection>(head)); }
void MotionScriptModule::applyArms(shared_ptr<ArmSection> arms) { applySection(dynamic_pointer_cast<BodySection>(arms)); }
void MotionScriptModule::applyLegs(shared_ptr<LegSection> legs) { applySection(dynamic_pointer_cast<BodySection>(legs)); }

bool MotionScriptModule::start(shared_ptr<MotionScriptRunner> scriptRunner)
{
  cout << "[MotionScriptModule::start] Starting script " << scriptRunner->getScriptName() << endl;

  if (d_runner && d_runner->getState() != MotionScriptRunnerState::Finished)
  {
    cerr << ccolor::warning
         << "[MotionScriptModule::start] Ignoring request to play script " << scriptRunner->getScriptName()
         << " -- already playing " << d_runner->getScriptName()
         << ccolor::reset
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
