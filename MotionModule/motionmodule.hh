#pragma once

#include <memory>

#include "../JointId/jointid.hh"
#include "../Configurable/configurable.hh"
#include "../MotionTask/motiontask.hh"

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
      d_name(type)
    {}
    
    virtual ~MotionModule() {}

    static const int TIME_UNIT = 8; //msec
    
    std::string getName() const { return d_name; }

    virtual void initialize() = 0;

    /// Updates the position. Returns false if the module considers this to be the
    /// final step required for the current MotionTask.
    virtual bool step(std::shared_ptr<JointSelection> selectedJoints) = 0;
    
    virtual void applyHead(std::shared_ptr<HeadSection> head) = 0;
    virtual void applyArms(std::shared_ptr<ArmSection> arms) = 0;
    virtual void applyLegs(std::shared_ptr<LegSection> legs) = 0;

    std::shared_ptr<MotionTaskScheduler> getScheduler() const { return d_scheduler; }
    
  private:
    std::shared_ptr<MotionTaskScheduler> d_scheduler;
    std::string d_name;
  };
}
