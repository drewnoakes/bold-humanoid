#pragma once

#include "../BodyPart/bodypart.hh"
#include "../JointId/jointid.hh"
#include "../Math/math.hh"

#include <memory>

namespace bold
{
  typedef unsigned char uchar;

  class JointControl
  {
  public:
    enum
    {
      P_GAIN_DEFAULT = 32,
      I_GAIN_DEFAULT = 0,
      D_GAIN_DEFAULT = 0
    };

    JointControl(unsigned jointId);

    uchar getId() const { return d_jointId; }

    void setValue(int value);
    int getValue() const { return d_value; }

    /// Sets the angle, in degrees
    void setAngle(double angle);
    /// Gets the angle, in degrees
    double getAngle() const { return d_angle; }

    /// Sets the angle, in radians
    void setRadian(double radian);
    /// Gets the angle, in radians
    double getRadian() { return Math::degToRad(getAngle()); }

    bool isDirty() const { return d_isDirty; }
    void clearDirty() { d_isDirty = false; }

    void setPGain(uchar p) { if (d_gainP == p) return; d_gainP = p; d_isDirty = true; }
    void setIGain(uchar i) { if (d_gainI == i) return; d_gainI = i; d_isDirty = true; }
    void setDGain(uchar d) { if (d_gainD == d) return; d_gainD = d; d_isDirty = true; }

    void setPidGains(uchar p, uchar i, uchar d) { setPGain(p); setIGain(i); setDGain(d); }

    uchar getPGain() const { return d_gainP; }
    uchar getIGain() const { return d_gainI; }
    uchar getDGain() const { return d_gainD; }
    
  private:
    uchar d_jointId;
    bool d_isDirty;
    int d_value;
    double d_angle;
    uchar d_gainP;
    uchar d_gainI;
    uchar d_gainD;
  };

  class HeadSection;
  class ArmSection;
  class LegSection;

  class BodyControl
  {
  private:
    const int JOINT_ARRAY_LENGTH = 21;

    std::vector<std::shared_ptr<JointControl>> d_joints;

    std::shared_ptr<HeadSection> d_headSection;
    std::shared_ptr<ArmSection> d_armSection;
    std::shared_ptr<LegSection> d_legSection;

  public:

    const uchar MIN_JOINT_ID = 1;
    const uchar MAX_JOINT_ID = 20;

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
      for (unsigned jointId = (int)d_minJointId; jointId <= (int)d_maxJointId; jointId++)
      {
        auto joint = d_body->getJoint((JointId)jointId);
        action(joint);
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