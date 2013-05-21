#include "bodycontrol.hh"

#include "../MX28/mx28.hh"

using namespace bold;
using namespace std;

BodyControl::BodyControl()
{
  for (int jointId = MIN_JOINT_ID; jointId <= MAX_JOINT_ID; jointId++)
    d_joints.push_back(make_shared<JointControl>(jointId));

  d_headSection = make_shared<HeadSection>(this);
  d_armSection = make_shared<ArmSection>(this);
  d_legSection = make_shared<LegSection>(this);
}

/////////////////////////////////////////////////////////////

JointControl::JointControl(unsigned jointId)
: d_jointId(jointId),
  d_isDirty(false),
  d_value(MX28::CENTER_VALUE),
  d_angle(0.0),
  d_gainP(P_GAIN_DEFAULT),
  d_gainI(I_GAIN_DEFAULT),
  d_gainD(D_GAIN_DEFAULT)
{}

void JointControl::setValue(int value)
{
  value = Math::clamp(value, MX28::MIN_VALUE, MX28::MAX_VALUE);
  if (d_value == value)
    return;
  d_value = value;
  d_angle = MX28::value2Degs(value);
  d_isDirty = true;
}

void JointControl::setAngle(double angle)
{
  angle = Math::clamp(angle, MX28::MIN_DEGS, MX28::MAX_DEGS);
  d_angle = angle;
  int value = MX28::degs2Value(angle);
  if (d_value == value)
    return;
  d_value = value;
  d_isDirty = true;
}
