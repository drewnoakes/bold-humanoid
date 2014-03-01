#include "motiontaskscheduler.ih"

void MotionTaskScheduler::update()
{
  assert(ThreadUtil::isThinkLoopThread());

  // Remove committed tasks for motion modules that have completed
  for (MotionModule* module : d_modules)
  {
    if (module->clearCompletedFlag())
    {
      // Remove any committed tasks for which the corresponding module has completed
      auto it1 = d_tasks.erase(
        remove_if(
          d_tasks.begin(), 
          d_tasks.end(),
          [module](shared_ptr<MotionTask> const& task) { return task->isCommitted() && task->getModule() == module; }
        ),
        d_tasks.end()
      );
      
      if (it1 != d_tasks.end())
      {
        // Something was removed
        d_hasChange = true;
      }
    }
  }
  
  if (!d_hasChange)
    return;

  // Determine which tasks are assigned which body sections
  auto moduleJointSelection = make_shared<vector<pair<MotionModule*, shared_ptr<JointSelection>>>>();
  
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
  
  MotionModule* headModule = headTask ? headTask->getModule() : nullptr;
  MotionModule* armModule = armTask ? armTask->getModule() : nullptr;
  MotionModule* legModule = legTask ? legTask->getModule() : nullptr;
  
  // This is a bit ugly, but I cannot think of a simpler way of grouping them in c++
  if (headModule == armModule && armModule == legModule)
  {
    // All sections controlled by same module
    if (headModule) moduleJointSelection->push_back(make_pair(headModule, JointSelection::all()));
  }
  else if (headModule == armModule)
  {
    // Head and arm sections controlled by same module
    if (headModule) moduleJointSelection->push_back(make_pair(headModule, JointSelection::headAndArms()));
    if (legModule)  moduleJointSelection->push_back(make_pair(legModule,  JointSelection::legs()));
  }
  else if (armModule == legModule)
  {
    // Arm and leg sections controlled by same module
    if (headModule) moduleJointSelection->push_back(make_pair(headModule, JointSelection::head()));
    if (armModule)  moduleJointSelection->push_back(make_pair(armModule,  JointSelection::armsAndLegs()));
  }
  else if (headModule == legModule)
  {
    // Head and leg sections controlled by same module
    if (headModule) moduleJointSelection->push_back(make_pair(headModule, JointSelection::headAndLegs()));
    if (armModule)  moduleJointSelection->push_back(make_pair(armModule,  JointSelection::arms()));
  }
  else
  {
    // Each section controlled by different modules
    if (headModule) moduleJointSelection->push_back(make_pair(headModule, JointSelection::head()));
    if (armModule)  moduleJointSelection->push_back(make_pair(armModule,  JointSelection::arms()));
    if (legModule)  moduleJointSelection->push_back(make_pair(legModule,  JointSelection::legs()));
  }
  
  // Commit selected tasks if they require it
  if (headTask && headTask->isCommitRequested()) headTask->setCommitted();
  if (armTask  && armTask->isCommitRequested())  armTask->setCommitted();
  if (legTask  && legTask->isCommitRequested())  legTask->setCommitted();
  
  // Generate motion task state
  State::set(make_shared<MotionTaskState const>(moduleJointSelection, headTasks, armTasks, legTasks));
  
  // Clear out non-committed tasks as they should only be presented to the
  // motion loop once. As the motion loop runs in 8ms vs the think loop at 30ms,
  // the stored MotionTaskState is guaranteed to be executed at least once.
  // Hence we can remove non-committed tasks now, readying this set of tasks
  // for the next think cycle.
  auto it2 = d_tasks.erase(
    remove_if(
      d_tasks.begin(), 
      d_tasks.end(),
      [](shared_ptr<MotionTask> task) { return !task->isCommitted(); }
    ),
    d_tasks.end()
  );
  
  d_hasChange = it2 == d_tasks.end();
}
