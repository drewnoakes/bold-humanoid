#pragma once

#include <memory>

#include "../JointId/jointid.hh"
#include "../util/assert.hh"

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
    High   = 3,
    Normal = 2,
    Low    = 1,
    None   = 0
  };

  enum class SectionId
  {
    Head,
    Arms,
    Legs
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
   * May also request to maintain control of the body section until some later
   * point, known as commital.
   *
   * This internal class is instantiated by the MotionTaskScheduler and is not
   * intended for direct use.
   */
  class MotionTask
  {
  public:
    MotionTask(MotionModule* module, SectionId section, Priority priority, bool isCommitRequested)
    : d_module(module),
      d_section(section),
      d_priority(priority),
      d_isCommitRequested(isCommitRequested),
      d_isCommitted(false)
    {}

    MotionModule* getModule() const { return d_module; }
    SectionId getSection() const { return d_section; }
    Priority getPriority() const { return d_priority; }
    bool isCommitRequested() const { return d_isCommitRequested; }
    bool isCommitted() const { return d_isCommitted; }

    /// Called by the framework when a task that requests committal is started.
    void setCommitted() { ASSERT(d_isCommitRequested); d_isCommitted = true; }

    /// For testing purposes
    void clearCommitted() { d_isCommitted = false; }
    void setPriority(Priority priority) { d_priority = priority; }

  private:
    MotionModule* d_module;
    SectionId d_section;
    Priority d_priority;
    bool d_isCommitRequested;
    bool d_isCommitted;
  };
}
