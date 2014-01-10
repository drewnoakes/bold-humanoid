#pragma once

#include "../BodyPart/bodypart.hh"
#include "../JointId/jointid.hh"
#include "../Math/math.hh"
#include "../util/Range.hh"

#include <memory>

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

  class JointControl
  {
  public:
    enum
    {
      P_GAIN_DEFAULT = 32,
      I_GAIN_DEFAULT = 0,
      D_GAIN_DEFAULT = 0
    };

    JointControl(uchar jointId);

    uchar getId() const { return d_jointId; }

    void setValue(ushort value);
    ushort getValue() const { return d_value; }

    /// Sets the target angle, in degrees
    void setDegrees(double degrees);
    /// Gets the target angle, in degrees
    double getDegrees() const { return d_degrees; }

    /// Sets the target angle, in radians
    void setRadians(double radian);
    /// Gets the target angle, in radians
    double getRadians() { return Math::degToRad(getDegrees()); }

    Range<int> getModifiedAddressRange() const { return d_changedAddressRange; }
    bool isDirty() const { return !d_changedAddressRange.isEmpty(); }
    void clearDirty() { d_changedAddressRange.reset(); }

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
    double d_degrees;
    uchar d_pGain;
    uchar d_iGain;
    uchar d_dGain;
    Range<int> d_changedAddressRange;
  };

  class HeadSection;
  class ArmSection;
  class LegSection;

  class BodyControl
  {
  private:
    std::vector<std::shared_ptr<JointControl>> d_joints;

    std::shared_ptr<HeadSection> d_headSection;
    std::shared_ptr<ArmSection> d_armSection;
    std::shared_ptr<LegSection> d_legSection;

  public:
    BodyControl();

    /** Sets all JointControl positions to match current hardware values. */
    void updateFromHardwareState();

    std::shared_ptr<JointControl> getJoint(JointId const id) const { return d_joints[(int)id - 1]; }

    std::vector<std::shared_ptr<JointControl>> getJoints() const { return d_joints; }

    std::shared_ptr<HeadSection> getHeadSection() const { return d_headSection; }
    std::shared_ptr<ArmSection>  getArmSection()  const { return d_armSection; }
    std::shared_ptr<LegSection>  getLegSection()  const { return d_legSection; }
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
        throw new std::runtime_error("Invalid min/max joint IDs");
    }

    void visitJoints(std::function<void(std::shared_ptr<JointControl>)> action)
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

    std::shared_ptr<JointControl> pan() const { return d_body->getJoint(JointId::HEAD_PAN); }
    std::shared_ptr<JointControl> tilt() const { return d_body->getJoint(JointId::HEAD_TILT); }
  };

  class ArmSection : public BodySection
  {
  public:
    ArmSection(BodyControl* body) : BodySection(body, JointId::R_SHOULDER_PITCH, JointId::L_ELBOW) {}

    std::shared_ptr<JointControl> shoulderPitchLeft() const { return d_body->getJoint(JointId::L_SHOULDER_PITCH); }
    std::shared_ptr<JointControl> shoulderPitchRight() const { return d_body->getJoint(JointId::R_SHOULDER_PITCH); }
    std::shared_ptr<JointControl> shoulderRollLeft() const { return d_body->getJoint(JointId::L_SHOULDER_ROLL); }
    std::shared_ptr<JointControl> shoulderRollRight() const { return d_body->getJoint(JointId::R_SHOULDER_ROLL); }
    std::shared_ptr<JointControl> elbowLeft() const { return d_body->getJoint(JointId::L_ELBOW); }
    std::shared_ptr<JointControl> elbowRight() const { return d_body->getJoint(JointId::R_ELBOW); }
  };

  class LegSection : public BodySection
  {
  public:
    LegSection(BodyControl* body) : BodySection(body, JointId::R_HIP_YAW, JointId::L_ANKLE_ROLL) {}

    std::shared_ptr<JointControl> hipYawLeft() const { return d_body->getJoint(JointId::L_HIP_YAW); }
    std::shared_ptr<JointControl> hipYawRight() const { return d_body->getJoint(JointId::R_HIP_YAW); }
    std::shared_ptr<JointControl> hipRollLeft() const { return d_body->getJoint(JointId::L_HIP_ROLL); }
    std::shared_ptr<JointControl> hipRollRight() const { return d_body->getJoint(JointId::R_HIP_ROLL); }
    std::shared_ptr<JointControl> hipPitchLeft() const { return d_body->getJoint(JointId::L_HIP_PITCH); }
    std::shared_ptr<JointControl> hipPitchRight() const { return d_body->getJoint(JointId::R_HIP_PITCH); }
    std::shared_ptr<JointControl> kneeLeft() const { return d_body->getJoint(JointId::L_KNEE); }
    std::shared_ptr<JointControl> kneeRight() const { return d_body->getJoint(JointId::R_KNEE); }
    std::shared_ptr<JointControl> anklePitchLeft() const { return d_body->getJoint(JointId::L_ANKLE_PITCH); }
    std::shared_ptr<JointControl> anklePitchRight() const { return d_body->getJoint(JointId::R_ANKLE_PITCH); }
    std::shared_ptr<JointControl> ankleRollLeft() const { return d_body->getJoint(JointId::L_ANKLE_ROLL); }
    std::shared_ptr<JointControl> ankleRollRight() const { return d_body->getJoint(JointId::R_ANKLE_ROLL); }
  };
}
