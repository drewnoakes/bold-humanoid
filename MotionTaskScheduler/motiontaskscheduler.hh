#pragma once

#include <memory>
#include <vector>
#include <mutex>

#include "../StateObject/MotionTaskState/motiontaskstate.hh"

namespace bold
{
  class MotionTask;
  class MotionModule;

  /** Tracks all active MotionTasks.
   *
   * Thread safe.
   */
  class MotionTaskScheduler
  {
  public:
    static void sortTasks(std::vector<std::shared_ptr<MotionTask>>& tasks);

    MotionTaskScheduler();

    void registerModule(MotionModule* module) { d_modules.push_back(module); }

    /** Enqueues motion tasks to be picked up in the next motion loop.
     *
     * Overall, the highest priority task per body section will be selected.
     * If the task has a commit request and is selected, it will be set
     * as committed until the corresponding MotionModule::step function
     * returns false.
     */
    void add(MotionModule* module,
             Priority headPriority, bool requestCommitHead,
             Priority armsPriority, bool requestCommitArms,
             Priority legsPriority, bool requestCommitLegs);

    /** Called at the end of each think loop, updating MotionTaskState.
     */
    void update();

  private:
    std::vector<std::shared_ptr<MotionTask>> d_tasks;
    std::vector<MotionModule*> d_modules;
    std::mutex d_mutex;
    bool d_hasChange;
  };
}
