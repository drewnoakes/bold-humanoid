#pragma once

#include <memory>
#include <vector>
#include <map>
#include <mutex>

#include "../MotionTask/motiontask.hh"
#include "../StateObject/MotionTaskState/motiontaskstate.hh"

namespace bold
{
  class MotionModule;

  enum class Required
  {
    /// If this body section cannot be controlled by this task, other body
    /// the task may still continue, controlling other sections.
    No,

    /// If this body section cannot be controlled by this task, the entire task
    /// cannot continue.
    Yes
  };

  enum class RequestCommit
  {
    /// The motion task, if selected, will only run for one think loop.
    /// Note that this equates to around four steps of the motion loop.
    No,

    /// The motion task, if selected, will run until the motion module
    /// clears its commit flag.
    Yes
  };

  /** Schedules tasks that control the motion of the robot's body sections.
   *
   * Thread-safe.
   */
  class MotionTaskScheduler
  {
  public:
    static void sortTasks(std::vector<std::shared_ptr<MotionTask>>& tasks);

    MotionTaskScheduler();

    void registerModule(MotionModule* module) { d_modules.push_back(module); }

    /** Enqueues a request to control body sections.
     *
     * Requests are divided by body section and processed separately.
     *
     * Priorities are used when more than one request is made during a cycle.
     *
     * If the task has a commit request and is selected, it will be set
     * as committed until the corresponding MotionModule clears the committed flag.
     *
     * If a body section is marked as required, then it must be selected in
     * order for non-required sections to also be selected. For example, if
     * a 'stand up' motion is added, then the arms and legs would be required
     * as it makes no sense for only the head to be selected.
     */
    std::shared_ptr<MotionRequest const> request(
      MotionModule* module,
      Priority headPriority, Required headRequired, RequestCommit headRequestCommit,
      Priority armsPriority, Required armsRequired, RequestCommit armsRequestCommit,
      Priority legsPriority, Required legsRequired, RequestCommit legsRequestCommit);

    /** Integrate any requests made since the last update, and check for completion of committed tasks.
     *
     * Called at the end of each think loop.
     *
     * Updates MotionTaskState.
     */
    void update();

    /** Gets the selected MotionTask for the head BodySection. May be nullptr. */
    std::shared_ptr<MotionTask> getHeadTask() const { return d_headTask; }
    /** Gets the selected MotionTask for the arm BodySection. May be nullptr. */
    std::shared_ptr<MotionTask> getArmTask()  const { return d_armTask; }
    /** Gets the selected MotionTask for the leg BodySection. May be nullptr. */
    std::shared_ptr<MotionTask> getLegTask()  const { return d_legTask; }

  private:
    std::vector<std::shared_ptr<MotionTask>> d_tasks;
    std::multimap<std::shared_ptr<MotionTask>,std::shared_ptr<MotionTask>> d_dependencies;
    std::shared_ptr<MotionTask> d_headTask;
    std::shared_ptr<MotionTask> d_armTask;
    std::shared_ptr<MotionTask> d_legTask;
    std::vector<MotionModule*> d_modules;
    std::mutex d_mutex;
    bool d_hasChange;
  };
}
