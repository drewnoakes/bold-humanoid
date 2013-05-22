#pragma once

#include <memory>

#include "../JointId/jointid.hh"
#include "../Configurable/configurable.hh"

namespace bold
{
  typedef unsigned char uchar;

  class HeadSection;
  class ArmSection;
  class LegSection;

  class JointSelection
  {
  public:
    JointSelection(bool head, bool arms, bool legs)
    {
      bool set = arms;
      for (uchar jointId = 1; jointId <= 20; jointId++)
      {
        if (jointId == (int)JointId::LEGS_START)
          set = legs;
        else if (jointId == (int)JointId::HEAD_START)
          set = head;

        d_set[jointId] = set;
      }
    }

    bool const& operator[] (uchar jointId) const { return d_set[jointId]; }

  private:
    bool d_set[21];
  };

  /** Abstract base for types of motion such as walking, running scripts or controlling the head.
   */
  class MotionModule : public Configurable
  {
  public:
    MotionModule(std::string const& type)
      : Configurable(std::string("motion.") + type)
    {}
    virtual ~MotionModule() {}

    static const int TIME_UNIT = 8; //msec

    virtual void initialize() = 0;

    /// Updates the position. Returns false if the module considers this to be the
    /// final step required for the current MotionTask.
    virtual bool step(JointSelection const& selectedJoints) = 0;
    virtual void applyHead(std::shared_ptr<HeadSection> head) = 0;
    virtual void applyArms(std::shared_ptr<ArmSection> arms) = 0;
    virtual void applyLegs(std::shared_ptr<LegSection> legs) = 0;
  };
}
