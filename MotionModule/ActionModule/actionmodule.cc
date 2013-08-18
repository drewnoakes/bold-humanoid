#include "actionmodule.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../MotionScriptFile/motionscriptfile.hh"
#include "../../MotionScriptPage/motionscriptpage.hh"
#include "../../MotionScriptRunner/motionscriptrunner.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../ThreadId/threadid.hh"

#include <cassert>
#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

ActionModule::ActionModule(std::shared_ptr<MotionTaskScheduler> scheduler, std::shared_ptr<MotionScriptFile> file)
: MotionModule("action", scheduler),
  d_file(file)
{
  assert(file);

  for (shared_ptr<MotionScriptPage> page : file->getSequenceRoots())
  {
    int pageIndex = file->indexOf(page);
    stringstream label;
    label << page->getName() << " (" << pageIndex << ")";
    cout << "[ActionModule::ActionModule] Found root page: " << label.str() << endl;
    d_controls.push_back(Control::createAction(label.str(), [this,pageIndex]() { start(pageIndex); }));
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

bool ActionModule::start(int pageIndex)
{
  auto page = d_file->getPageByIndex(pageIndex);

  return page ? start(pageIndex, page) : false;
}

bool ActionModule::start(string const& pageName)
{
  for (int index = 0; index < MotionScriptFile::MAX_PAGE_ID; index++)
  {
    auto page = d_file->getPageByIndex(index);
    if (page->getName() == pageName)
      return start(index, page);
  }

  cerr << "[ActionModule::start] No page with name " << pageName << " found" << endl;
  return false;
}

bool ActionModule::start(int index, shared_ptr<MotionScriptPage> page)
{
  cout << "[ActionModule::start] Starting page index " << index << " (" << page->getName() << ")" << endl;
  
  if (d_runner && d_runner->getState() != MotionScriptRunnerState::Finished)
  {
    cerr << "[ActionModule::start] Ignoring request to play page " << index << " -- already playing page " << d_runner->getCurrentPageIndex() << endl;
    return false;
  }

  if (page->getRepeatCount() == 0 || page->getStepCount() == 0)
  {
    cerr << "[ActionModule::start] Page at index " << index << " has no steps to perform" << endl;
    return false;
  }

  d_runner = make_shared<MotionScriptRunner>(d_file, page, index);

  getScheduler()->add(this,
                      Priority::Optional,  true,  // HEAD   Interuptable::YES
                      Priority::Important, true,  // ARMS
                      Priority::Important, true); // LEGS

  return true;
}
