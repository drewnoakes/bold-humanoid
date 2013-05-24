#include "motiontaskscheduler.hh"

#include "../MotionModule/motionmodule.hh"

#include <algorithm>

using namespace bold;
using namespace std;

void MotionTaskScheduler::update()
{
  // Remove all non-committed tasks
  auto it1 = d_tasks.erase(
    remove_if(
      d_tasks.begin(), 
      d_tasks.end(),
      [](shared_ptr<MotionTask> const& task) { return !task->isCommitted(); }
    )
  );
  
  if (it1 != d_tasks.end())
    d_hasChange = true; // something was removed

  // Remove committed tasks for motion modules that have completed
  for (MotionModule* module : d_modules)
  {
    if (module->clearCompletedFlag())
    {
      // Remove any committed tasks for which the corresponding module has completed
      auto it2 = d_tasks.erase(
        remove_if(
          d_tasks.begin(), 
          d_tasks.end(),
          [module](shared_ptr<MotionTask> const& task) { return task->isCommitted() && task->getModule() == module; }
        )
      );
      
      if (it2 != d_tasks.end())
      {
        // Something was removed
        d_hasChange = true;
      }
    }
  }
  
  if (!d_hasChange)
    return;

  // Determine which tasks are assigned which body sections
  auto moduleJointSelection = make_shared<vector<pair<shared_ptr<MotionTask>, shared_ptr<JointSelection>>>>();
  
  vector<shared_ptr<MotionTask>> headTasks;
  vector<shared_ptr<MotionTask>> armTasks;
  vector<shared_ptr<MotionTask>> legTasks;
  
  for (shared_ptr<MotionTask> task : d_tasks)
  {
    if (task->getSection() == SectionId::Head)
      headTasks.push_back(task);
    if (task->getSection() == SectionId::Arms)
      armTasks.push_back(task);
    if (task->getSection() == SectionId::Legs)
      legTasks.push_back(task);
  }
  
  auto sortTasks = [](vector<shared_ptr<MotionTask>>& tasks)
  {
    std::stable_sort(
      tasks.begin(), 
      tasks.end(),
      [](shared_ptr<MotionTask> a, shared_ptr<MotionTask> b)
      {
        // Committed first
        if (a->isCommitted() && !b->isCommitted())
          return true;
        
        // Sort larger priorities first
        return a->getPriority() >= b->getPriority();
      }
    );
  };
  
  sortTasks(headTasks);
  sortTasks(armTasks);
  sortTasks(legTasks);
  
  shared_ptr<MotionTask> headTask = headTasks.size() ? headTasks[0] : nullptr;
  shared_ptr<MotionTask> armTask  = armTasks.size()  ? armTasks[0] : nullptr;
  shared_ptr<MotionTask> legTask  = legTasks.size()  ? legTasks[0] : nullptr;
  
  // This is a bit ugly, but I cannot think of a simpler way of grouping them in c++
  if (headTask == armTask && armTask == legTask)
  {
    if (headTask) moduleJointSelection->push_back(make_pair(headTask, JointSelection::all()));
  }
  else if (headTask == armTask)
  {
    if (headTask) moduleJointSelection->push_back(make_pair(headTask, JointSelection::headAndArms()));
    if (legTask)  moduleJointSelection->push_back(make_pair(legTask,  JointSelection::legs()));
  }
  else if (armTask == legTask)
  {
    if (headTask) moduleJointSelection->push_back(make_pair(headTask, JointSelection::head()));
    if (armTask)  moduleJointSelection->push_back(make_pair(armTask,  JointSelection::armsAndLegs()));
  }
  else if (headTask == legTask)
  {
    if (headTask) moduleJointSelection->push_back(make_pair(headTask, JointSelection::headAndLegs()));
    if (armTask)  moduleJointSelection->push_back(make_pair(armTask,  JointSelection::arms()));
  }
  else
  {
    if (headTask) moduleJointSelection->push_back(make_pair(headTask, JointSelection::head()));
    if (armTask)  moduleJointSelection->push_back(make_pair(armTask,  JointSelection::arms()));
    if (legTask)  moduleJointSelection->push_back(make_pair(legTask,  JointSelection::legs()));
  }
  
  // Commit selected tasks if they require it
  if (headTask && headTask->isCommitRequested()) headTask->setCommitted();
  if (armTask  && armTask->isCommitRequested())  armTask->setCommitted();
  if (legTask  && legTask->isCommitRequested())  legTask->setCommitted();
  
  // Generate motion task state
  AgentState::getInstance().set(make_shared<MotionTaskState const>(moduleJointSelection, headTasks, armTasks, legTasks));
  
  // Clear out non-committed tasks
  d_tasks.erase(
    remove_if(
      d_tasks.begin(), 
      d_tasks.end(),
      [](shared_ptr<MotionTask> task) { return !task->isCommitted(); }
    )
  );
  
  d_hasChange = false;
}
