#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <mutex>

#include "../JointId/jointid.hh"
#include "../MotionTask/motiontask.hh"
#include "../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../ThreadUtil/threadutil.hh"

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;
  typedef unsigned long ulong;

  class ArmSection;
  class HeadSection;
  class MotionTaskScheduler;
  class LegSection;

  /** Abstract base for types of motion such as walking, running scripts or controlling the head.
   */
  class MotionModule
  {
  public:
    MotionModule(std::string const& type, std::shared_ptr<MotionTaskScheduler> scheduler)
    : d_scheduler(scheduler),
      d_name(type),
      d_isCompleted(false)
    {
      d_scheduler->registerModule(this);
    }

    virtual ~MotionModule() = default;

    static constexpr int TIME_UNIT = 8; //msec

    std::string getName() const { return d_name; }

    /** Called once, when the module is added to the MotionLoop. */
    virtual void initialize() = 0;

    /** Updates the position.
     *
     * @param selectedJoints indicates which body sections and joints may be
     *                       controlled.
     */
    virtual void step(std::shared_ptr<JointSelection> selectedJoints) = 0;

    virtual void applyHead(HeadSection* head) = 0;
    virtual void applyArms(ArmSection* arms) = 0;
    virtual void applyLegs(LegSection* legs) = 0;

    std::shared_ptr<MotionTaskScheduler> getScheduler() const { return d_scheduler; }

    // The flag will be set from one thread, and cleared from another,
    // but a single field write/read should be thread safe.

    /// Called by the motion module (on the motion loop thread) when the module
    /// has completed. For example, when a motion script has reached its final
    /// pose, or when walking has come to a stop.
    void setCompletedFlag()
    {
      assert(ThreadUtil::isMotionLoopThread());

      std::lock_guard<std::mutex> guard(d_isCompletedMutex);
      d_isCompleted = true;
    }

    /// Called by the MotionTaskScheduler (on the think thread) to test whether
    /// the module completed since the time this method was called. For each call
    /// to setCompletedFlag, this method will return true once.
    /// When true is returned, any motion tasks associated with this module that
    /// were committed will be removed from the schedule.
    bool clearCompletedFlag()
    {
      assert(ThreadUtil::isThinkLoopThread());

      std::lock_guard<std::mutex> guard(d_isCompletedMutex);
      bool isSet = d_isCompleted;
      d_isCompleted = false;
      return isSet;
    }

  private:
    MotionModule(const MotionModule&) = delete;
    MotionModule& operator=(const MotionModule&) = delete;

    std::mutex d_isCompletedMutex;
    std::shared_ptr<MotionTaskScheduler> d_scheduler;
    std::string d_name;
    bool d_isCompleted;
  };
}
