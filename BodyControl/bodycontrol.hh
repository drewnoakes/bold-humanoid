#pragma once

#include "../BodyPart/bodypart.hh"
#include "../JointId/jointid.hh"
#include "../Math/math.hh"
#include "../util/Range.hh"
#include "../MX28/mx28.hh"

#include <memory>

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

  class JointControl
  {
  public:
    static constexpr uchar DefaultPGain = 32;
    static constexpr uchar DefaultIGain = 0;
    static constexpr uchar DefaultDGain = 0;

    JointControl(uchar jointId);

    uchar getId() const { return d_jointId; }

    void setValue(ushort value);
    ushort getValue() const { return d_value; }

    void setModulationOffset(short delta);
    short getModulationOffset() const { return d_modulationOffset; }

    /// Sets the target angle, in degrees
    void setDegrees(double degrees);
    /// Gets the target angle, in degrees
    double getDegrees() const { return d_degrees; }

    /// Sets the target angle, in radians
    void setRadians(double radian);
    /// Gets the target angle, in radians
    double getRadians() { return Math::degToRad(getDegrees()); }

    Range<MX28Table> getModifiedAddressRange() const { return d_changedAddressRange; }
    bool isDirty() const { return !d_changedAddressRange.isEmpty(); }
    void clearDirty() { d_changedAddressRange.reset(); }

    void notifyOffsetChanged();

    void setPGain(uchar p);
    void setIGain(uchar i);
    void setDGain(uchar d);

    void setPidGains(uchar p, uchar i, uchar d);

    uchar getPGain() const { return d_pGain; }
    uchar getIGain() const { return d_iGain; }
    uchar getDGain() const { return d_dGain; }

  private:
    uchar d_jointId;
    ushort d_value;
    short d_modulationOffset;
    double d_degrees;
    uchar d_pGain;
    uchar d_iGain;
    uchar d_dGain;
    Range<MX28Table> d_changedAddressRange;
  };

  class ArmSection;
  class HeadSection;
  class LegSection;
  class HardwareState;

  class BodyControl
  {
  public:
    BodyControl();

    /** Sets all JointControl positions to match current hardware values. */
    void updateFromHardwareState(std::shared_ptr<HardwareState const> const& hw);

    JointControl* getJoint(JointId const id) const { return d_joints[(int)id - 1].get(); }

    /// Gets a vector of all joints in ID order, starting with JointId 1 in position 0.
    std::vector<std::unique_ptr<JointControl>> const& getJoints() const { return d_joints; }

    HeadSection* getHeadSection() const { return d_headSection.get(); }
    ArmSection*  getArmSection()  const { return d_armSection.get(); }
    LegSection*  getLegSection()  const { return d_legSection.get(); }

    void clearModulation();
    void modulateJoint(JointId id, short delta);

  private:
    std::vector<std::unique_ptr<JointControl>> d_joints;
    std::vector<int> d_modulationOffsets;

    std::unique_ptr<HeadSection> d_headSection;
    std::unique_ptr<ArmSection> d_armSection;
    std::unique_ptr<LegSection> d_legSection;
  };

  class BodySection
  {
  public:
    BodySection(BodyControl* body, JointId minJointId, JointId maxJointId)
    : d_body(body),
      d_minJointId(minJointId),
      d_maxJointId(maxJointId)
    {
      if ((int)minJointId > (int)maxJointId)
        throw std::runtime_error("Invalid min/max joint IDs");
    }

    void visitJoints(std::function<void(JointControl*)> action)
    {
      for (uchar jointId = (uchar)d_minJointId; jointId <= (uchar)d_maxJointId; jointId++)
      {
        action(d_body->getJoint((JointId)jointId));
      }
    }

  protected:
    BodyControl* d_body;
    JointId d_minJointId;
    JointId d_maxJointId;
  };

  class HeadSection : public BodySection
  {
  public:
    HeadSection(BodyControl* body) : BodySection(body, JointId::HEAD_PAN, JointId::HEAD_TILT) {}

    JointControl* pan() const { return d_body->getJoint(JointId::HEAD_PAN); }
    JointControl* tilt() const { return d_body->getJoint(JointId::HEAD_TILT); }
  };

  class ArmSection : public BodySection
  {
  public:
    ArmSection(BodyControl* body) : BodySection(body, JointId::R_SHOULDER_PITCH, JointId::L_ELBOW) {}

    JointControl* shoulderPitchLeft() const { return d_body->getJoint(JointId::L_SHOULDER_PITCH); }
    JointControl* shoulderPitchRight() const { return d_body->getJoint(JointId::R_SHOULDER_PITCH); }
    JointControl* shoulderRollLeft() const { return d_body->getJoint(JointId::L_SHOULDER_ROLL); }
    JointControl* shoulderRollRight() const { return d_body->getJoint(JointId::R_SHOULDER_ROLL); }
    JointControl* elbowLeft() const { return d_body->getJoint(JointId::L_ELBOW); }
    JointControl* elbowRight() const { return d_body->getJoint(JointId::R_ELBOW); }
  };

  class LegSection : public BodySection
  {
  public:
    LegSection(BodyControl* body) : BodySection(body, JointId::R_HIP_YAW, JointId::L_ANKLE_ROLL) {}

    JointControl* hipYawLeft() const { return d_body->getJoint(JointId::L_HIP_YAW); }
    JointControl* hipYawRight() const { return d_body->getJoint(JointId::R_HIP_YAW); }
    JointControl* hipRollLeft() const { return d_body->getJoint(JointId::L_HIP_ROLL); }
    JointControl* hipRollRight() const { return d_body->getJoint(JointId::R_HIP_ROLL); }
    JointControl* hipPitchLeft() const { return d_body->getJoint(JointId::L_HIP_PITCH); }
    JointControl* hipPitchRight() const { return d_body->getJoint(JointId::R_HIP_PITCH); }
    JointControl* kneeLeft() const { return d_body->getJoint(JointId::L_KNEE); }
    JointControl* kneeRight() const { return d_body->getJoint(JointId::R_KNEE); }
    JointControl* anklePitchLeft() const { return d_body->getJoint(JointId::L_ANKLE_PITCH); }
    JointControl* anklePitchRight() const { return d_body->getJoint(JointId::R_ANKLE_PITCH); }
    JointControl* ankleRollLeft() const { return d_body->getJoint(JointId::L_ANKLE_ROLL); }
    JointControl* ankleRollRight() const { return d_body->getJoint(JointId::R_ANKLE_ROLL); }
  };
}
