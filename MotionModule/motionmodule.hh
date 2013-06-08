#pragma once

#include <cassert>
#include <memory>
#include <string>

#include "../JointId/jointid.hh"
#include "../Configurable/configurable.hh"
#include "../MotionTask/motiontask.hh"
#include "../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../ThreadId/threadid.hh"

namespace bold
{
  typedef unsigned char uchar;

  class ArmSection;
  class HeadSection;
  class MotionTaskScheduler;
  class LegSection;

  /** Abstract base for types of motion such as walking, running scripts or controlling the head.
   */
  class MotionModule : public Configurable
  {
  public:
    MotionModule(std::string const& type, std::shared_ptr<MotionTaskScheduler> scheduler)
    : Configurable(std::string("motion.") + type),
      d_scheduler(scheduler),
      d_name(type),
      d_isCompleted(false)
    {
      d_scheduler->registerModule(this);
    }
    
    virtual ~MotionModule() {}

    static const int TIME_UNIT = 8; //msec
    
    std::string getName() const { return d_name; }

    virtual void initialize() = 0;

    /** Updates the position.
     * 
     * @param selectedJoints indicates which body sections and joints may be
     *                       controlled.
     */
    virtual void step(std::shared_ptr<JointSelection> selectedJoints) = 0;
    
    virtual void applyHead(std::shared_ptr<HeadSection> head) = 0;
    virtual void applyArms(std::shared_ptr<ArmSection> arms) = 0;
    virtual void applyLegs(std::shared_ptr<LegSection> legs) = 0;

    std::shared_ptr<MotionTaskScheduler> getScheduler() const { return d_scheduler; }

    // The flag will be set from on thread, and cleared from another,
    // but a single field write/read should be thread safe.
    
    void setCompletedFlag()
    {
      assert(ThreadId::isMotionLoopThread());
      
      std::lock_guard<std::mutex> guard(d_isCompletedMutex);
      d_isCompleted = true; 
    }
    
    bool clearCompletedFlag()
    {
      assert(ThreadId::isThinkLoopThread());
      
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
