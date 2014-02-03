#pragma once

#include <cassert>
#include <memory>
#include <string>

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

    virtual ~MotionModule() {}

    MotionModule(const MotionModule&) = delete;
    MotionModule& operator=(const MotionModule&) = delete;

    static const int TIME_UNIT = 8; //msec

    std::string getName() const { return d_name; }

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

    // The flag will be set from on thread, and cleared from another,
    // but a single field write/read should be thread safe.

    void setCompletedFlag()
    {
      assert(ThreadUtil::isMotionLoopThread());

      std::lock_guard<std::mutex> guard(d_isCompletedMutex);
      d_isCompleted = true;
    }

    bool clearCompletedFlag()
    {
      assert(ThreadUtil::isThinkLoopThread());

      std::lock_guard<std::mutex> guard(d_isCompletedMutex);
      bool isSet = d_isCompleted;
      d_isCompleted = false;
      return isSet;
    }

  private:
    std::mutex d_isCompletedMutex;
    std::shared_ptr<MotionTaskScheduler> d_scheduler;
    std::string d_name;
    bool d_isCompleted;
  };
}
