#pragma once

#include <memory>

#include "../JointId/jointid.hh"

namespace bold
{
  class MotionModule;
  
  typedef unsigned char uchar;

  /** The priority of a motion task.
   * 
   * Used in arbitrating which motion module gains control of a body section.
   */
  enum class Priority
  {
    Important = 2,
    Normal    = 1,
    Optional  = 0
  };

  /** Models a set of joints.
   */
  class JointSelection
  {
  public:
    JointSelection(bool head, bool arms, bool legs)
    : d_head(head),
      d_arms(arms),
      d_legs(legs)
    {
      bool set = arms;
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if (jointId == (int)JointId::LEGS_START)
          set = legs;
        else if (jointId == (int)JointId::HEAD_START)
          set = head;

        d_set[jointId] = set;
      }
    }

    bool const& operator[] (uchar jointId) const { return d_set[jointId]; }
    
    bool hasHead() const { return d_head; }
    bool hasArms() const { return d_arms; }
    bool hasLegs() const { return d_legs; }
    
    static std::shared_ptr<JointSelection> const& all()         { static auto instance = std::make_shared<JointSelection>(true,  true,  true);  return instance; }
    static std::shared_ptr<JointSelection> const& head()        { static auto instance = std::make_shared<JointSelection>(true,  false, false); return instance; }
    static std::shared_ptr<JointSelection> const& arms()        { static auto instance = std::make_shared<JointSelection>(false, true,  false); return instance; }
    static std::shared_ptr<JointSelection> const& legs()        { static auto instance = std::make_shared<JointSelection>(false, false, true);  return instance; }
    static std::shared_ptr<JointSelection> const& headAndArms() { static auto instance = std::make_shared<JointSelection>(true,  true,  false); return instance; }
    static std::shared_ptr<JointSelection> const& headAndLegs() { static auto instance = std::make_shared<JointSelection>(true,  false, true);  return instance; }
    static std::shared_ptr<JointSelection> const& armsAndLegs() { static auto instance = std::make_shared<JointSelection>(false, true,  true);  return instance; }

  private:
    bool d_set[21];
    bool d_head;
    bool d_arms;
    bool d_legs;
  };
 
  /** Represents a desire to control a body section using a particular motion module.
   * 
   * May also request to maintain control of the body section until some later point.
   */
  class MotionTask
  {
  public:
    MotionTask(MotionModule* module, std::shared_ptr<JointSelection> jointSelection, Priority priority, bool isCommitRequested)
    : d_module(module),
      d_jointSelection(jointSelection),
      d_priority(priority),
      d_isCommitRequested(isCommitRequested),
      d_isCommitted(false)
    {}
    
    MotionModule* getModule() const { return d_module; }
    std::shared_ptr<JointSelection> getJointSelection() const { return d_jointSelection; }
    Priority getPriority() const { return d_priority; }
    bool isCommitRequested() const { return d_isCommitRequested; }
    bool isCommitted() const { return d_isCommitted; }
    
    /// Called by the framework when a task that requests committal is started.
    void setCommitted()
    {
      d_isCommitted = true;
    }
  
  private:
    MotionModule* d_module;
    std::shared_ptr<JointSelection> d_jointSelection;
    Priority d_priority;
    bool d_isCommitRequested;
    bool d_isCommitted;
  };  
}