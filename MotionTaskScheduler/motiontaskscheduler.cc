#include "motiontaskscheduler.hh"

#include "../MotionModule/motionmodule.hh"
#include "../MotionTask/motiontask.hh"
#include "../State/state.hh"
#include "../ThreadUtil/threadutil.hh"
#include "../util/assert.hh"

#include <algorithm>
#include <set>

using namespace bold;
using namespace std;

string getSectionIdName(SectionId section)
{
  switch (section)
  {
    case SectionId::Head: return "Head";
    case SectionId::Arms: return "Arms";
    case SectionId::Legs: return "Legs";
    default: return "Unknown";
  }
}

////////////////////////////////////////////////////////////////////////////////

MotionTaskScheduler::MotionTaskScheduler()
: d_modules(),
  d_hasChange(false)
{}

shared_ptr<MotionRequest const> MotionTaskScheduler::request(
  MotionModule* module,
  Priority headPriority, Required headRequired, RequestCommit headRequestCommit,
  Priority armsPriority, Required armsRequired, RequestCommit armsRequestCommit,
  Priority legsPriority, Required legsRequired, RequestCommit legsRequestCommit)
{
  lock_guard<mutex> guard(d_mutex);

  auto request = make_shared<MotionRequest>();

  auto handleSection = [this,module,&request](SectionId section, Priority priority, RequestCommit requestCommit) -> shared_ptr<MotionTask>
  {
    if (priority == Priority::None)
      return nullptr;

    auto task = make_shared<MotionTask>(request, module, section, priority, requestCommit == RequestCommit::Yes);
    d_tasks.push_back(task);
    d_hasChange = true;
    request->setSectionTask(section, task);
    return task;
  };

  auto headTask = handleSection(SectionId::Head, headPriority, headRequestCommit);
  auto armTask  = handleSection(SectionId::Arms, armsPriority, armsRequestCommit);
  auto legTask  = handleSection(SectionId::Legs, legsPriority, legsRequestCommit);

  auto linkDependencies = [this](Required required, shared_ptr<MotionTask> const& requiredTask, shared_ptr<MotionTask> const& depTask1, shared_ptr<MotionTask> const& depTask2)
  {
    if (requiredTask == nullptr || required != Required::Yes)
      return;

    if (depTask1 != nullptr)
      d_dependencies.insert(make_pair(depTask1, requiredTask));

    if (depTask2 != nullptr)
      d_dependencies.insert(make_pair(depTask2, requiredTask));
  };

  linkDependencies(headRequired, headTask, armTask, legTask);
  linkDependencies(armsRequired, armTask,  headTask, legTask);
  linkDependencies(legsRequired, legTask,  headTask, armTask);

  return request;
}

void MotionTaskScheduler::update()
{
  ASSERT(ThreadUtil::isThinkLoopThread());

  lock_guard<mutex> guard(d_mutex);

  // Remove committed tasks for motion modules that have completed
  for (MotionModule* module : d_modules)
  {
    if (module->clearCompletedFlag())
    {
      // Remove any committed tasks for which the corresponding module has completed
      d_tasks.erase(
        remove_if(
          d_tasks.begin(),
          d_tasks.end(),
          [this,module](shared_ptr<MotionTask> const& task)
          {
            if (task->isCommitted() && task->getModule() == module)
            {
              // Set dirty state
              d_hasChange = true;
              // Notify the task that it has completed
              task->setCompleted();
              return true;
            }
            return false;
          }
        ),
        d_tasks.end()
      );
    }
  }

  if (!d_hasChange)
    return;

  //
  // Determine which tasks are assigned which body sections
  //

  vector<shared_ptr<MotionTask>> tasks = d_tasks;

  sortTasks(tasks);

  shared_ptr<MotionTask> headTask;
  shared_ptr<MotionTask> armsTask;
  shared_ptr<MotionTask> legsTask;

  auto selectTask = [&](shared_ptr<MotionTask> const& task)
  {
    task->setSelected();

    auto section = task->getSection();

    switch (section)
    {
      case SectionId::Head: ASSERT(headTask == nullptr || headTask == task); headTask = task; break;
      case SectionId::Arms: ASSERT(armsTask == nullptr || armsTask == task); armsTask = task; break;
      case SectionId::Legs: ASSERT(legsTask == nullptr || legsTask == task); legsTask = task; break;
    }

    tasks.erase(
      remove_if(
        tasks.begin(),
        tasks.end(),
        [&task,section](shared_ptr<MotionTask> const& t)
        {
          if (t->getSection() == section)
          {
            if (t != task)
              t->setIgnored();
            return true;
          }
          return false;
        }
      ),
      tasks.end()
    );
  };

  while (!tasks.empty() && (!headTask || !armsTask || !legsTask))
  {
    auto first = tasks[0]; // NOTE deliberately not a reference!

    if (first->isCommitted())
    {
      // If the task is committed, it is selected
      selectTask(first);
    }
    else
    {
      // Otherwise, we select this task if it has no dependencies, or its
      // dependencies can also be selected.

      auto dependencies = d_dependencies.equal_range(first);

      if (dependencies.first == dependencies.second)
      {
        // No dependencies, so just apply it
        selectTask(first);
      }
      else
      {
        // Only select this task if its dependencies may also be selected
        bool canSelect = true;
        for (auto dep = dependencies.first; dep != dependencies.second; dep++)
        {
          auto depSection = dep->second->getSection();
          if (depSection == SectionId::Head && (headTask != nullptr && headTask != dep->second))
            canSelect = false;
          else if (depSection == SectionId::Arms && (armsTask != nullptr && armsTask != dep->second))
            canSelect = false;
          else if (depSection == SectionId::Legs && (legsTask != nullptr && legsTask != dep->second))
            canSelect = false;
        }

        if (canSelect)
        {
          selectTask(first);
          for (auto dep = dependencies.first; dep != dependencies.second; dep++)
            selectTask(dep->second);
        }
        else
        {
          // Set the task and its dependencies as ignored
          first->setIgnored();
          for (auto dep = dependencies.first; dep != dependencies.second; dep++)
            dep->second->setIgnored();
        }
      }

      if (!tasks.empty() && tasks[0].get() == first.get())
        tasks.erase(tasks.begin());
    }
  }

  d_headTask = headTask;
  d_armTask  = armsTask;
  d_legTask  = legsTask;

  //////////

  MotionModule* headModule = d_headTask ? d_headTask->getModule() : nullptr;
  MotionModule* armModule  = d_armTask  ? d_armTask->getModule() : nullptr;
  MotionModule* legModule  = d_legTask  ? d_legTask->getModule() : nullptr;

  auto moduleJointSelection = make_shared<vector<pair<MotionModule*, shared_ptr<JointSelection>>>>();

  // This is a bit ugly, but I cannot think of a simpler way of grouping them in c++
  if (headModule == armModule && armModule == legModule)
  {
    // All sections controlled by same module
    if (headModule) moduleJointSelection->emplace_back(headModule, JointSelection::all());
  }
  else if (headModule == armModule)
  {
    // Head and arm sections controlled by same module
    if (headModule) moduleJointSelection->emplace_back(headModule, JointSelection::headAndArms());
    if (legModule)  moduleJointSelection->emplace_back(legModule,  JointSelection::legs());
  }
  else if (armModule == legModule)
  {
    // Arm and leg sections controlled by same module
    if (headModule) moduleJointSelection->emplace_back(headModule, JointSelection::head());
    if (armModule)  moduleJointSelection->emplace_back(armModule,  JointSelection::armsAndLegs());
  }
  else if (headModule == legModule)
  {
    // Head and leg sections controlled by same module
    if (headModule) moduleJointSelection->emplace_back(headModule, JointSelection::headAndLegs());
    if (armModule)  moduleJointSelection->emplace_back(armModule,  JointSelection::arms());
  }
  else
  {
    // Each section controlled by different modules
    if (headModule) moduleJointSelection->emplace_back(headModule, JointSelection::head());
    if (armModule)  moduleJointSelection->emplace_back(armModule,  JointSelection::arms());
    if (legModule)  moduleJointSelection->emplace_back(legModule,  JointSelection::legs());
  }

  // Commit selected tasks if they require it
  if (d_headTask && d_headTask->isCommitRequested()) d_headTask->setCommitted();
  if (d_armTask  && d_armTask->isCommitRequested())  d_armTask->setCommitted();
  if (d_legTask  && d_legTask->isCommitRequested())  d_legTask->setCommitted();

  // Generate motion task state
  State::make<MotionTaskState>(moduleJointSelection, headTask, armsTask, legsTask, d_tasks);

  // Clear out non-committed tasks as they should only be presented to the
  // motion loop once. As the motion loop runs in 8ms vs the think loop at 30ms,
  // the stored MotionTaskState is guaranteed to be executed at least once.
  // Hence we can remove non-committed tasks now, readying this set of tasks
  // for the next think cycle.
  d_tasks.erase(
    remove_if(
      d_tasks.begin(),
      d_tasks.end(),
      [this](shared_ptr<MotionTask> const& task)
      {
        if (!task->isCommitted() && task->getStatus() == MotionTaskStatus::Selected)
        {
          d_hasChange = true;
          task->setCompleted();
          return true;
        }
        return false;
      }
    ),
    d_tasks.end()
  );

  d_dependencies.clear();
}

void MotionTaskScheduler::sortTasks(vector<shared_ptr<MotionTask>>& tasks)
{
  std::stable_sort(
    tasks.begin(),
    tasks.end(),
    [](shared_ptr<MotionTask> const& a, shared_ptr<MotionTask> const& b)
    {
      // Committed first
      if (a->isCommitted() && !b->isCommitted())
        return true;

      // Sort higher priorities first
      return a->getPriority() > b->getPriority();
    }
  );

#ifdef INCLUDE_ASSERTIONS
  // Ensure we never end up with more than one committed task per section
  set<SectionId> committedSections;

  for (auto const& task : tasks)
  {
    if (!task->isCommitted())
      continue;

    auto section = task->getSection();
    auto it = committedSections.find(section);

    if (it != committedSections.end())
    {
      log::error("MotionTaskScheduler::sortTasks") << "Multiple tasks committed for section: " << getSectionIdName(section);
      throw runtime_error("Multiple tasks committed for same body section");
    }

    committedSections.insert(section);
  }
#endif
}
