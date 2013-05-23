#include "motiontaskscheduler.hh"

#include <algorithm>

using namespace bold;
using namespace std;

void MotionTaskScheduler::update()
{
  if (!d_hasChange)
    return;
  
  lock_guard<mutex> g(d_mutex);

  // Double check once lock acquired
  if (!d_hasChange)
  {
    cerr << "[MotionTaskScheduler::update] Unexpected concurrent calls detected" << endl;
    return;
  }

  // Determine which tasks are assigned which body sections
  auto moduleJointSelection = make_shared<vector<pair<shared_ptr<MotionTask>, shared_ptr<JointSelection>>>>();
  
  vector<shared_ptr<MotionTask>> headTasks;
  vector<shared_ptr<MotionTask>> armTasks;
  vector<shared_ptr<MotionTask>> legTasks;
  
  for (shared_ptr<MotionTask> task : d_tasks)
  {
    if (task->getJointSelection()->hasHead())
      headTasks.push_back(task);
    if (task->getJointSelection()->hasArms())
      armTasks.push_back(task);
    if (task->getJointSelection()->hasLegs())
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
  shared_ptr<MotionTask> armTask = armTasks.size() ? armTasks[0] : nullptr;
  shared_ptr<MotionTask> legTask = legTasks.size() ? legTasks[0] : nullptr;
  
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
    remove_if(d_tasks.begin(), d_tasks.end(), [](shared_ptr<MotionTask> const& task) { return !task->isCommitted(); }),
    d_tasks.end());
  
  d_hasChange = false;
}
