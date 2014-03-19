#include "bodycontrol.hh"

#include "../MX28/mx28.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../State/state.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"

using namespace bold;
using namespace std;

BodyControl::BodyControl()
{
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    d_joints.push_back(make_unique<JointControl>(jointId));

  d_headSection = make_unique<HeadSection>(this);
  d_armSection  = make_unique<ArmSection>(this);
  d_legSection  = make_unique<LegSection>(this);
}

void BodyControl::updateFromHardwareState(shared_ptr<HardwareState const> const& hw)
{
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    JointControl* joint = getJoint((JointId)jointId);

    // Set measured hardware position
    joint->setValue(hw->getMX28State(jointId).presentPositionValue);

    // Clear dirty flag. Value came from hardware, so no need to write it back again.
    joint->clearDirty();
  }
}

/////////////////////////////////////////////////////////////

JointControl::JointControl(uchar jointId)
: d_jointId(jointId),
  d_value(MX28::CENTER_VALUE),
  d_degrees(0.0),
  d_pGain(P_GAIN_DEFAULT),
  d_iGain(I_GAIN_DEFAULT),
  d_dGain(D_GAIN_DEFAULT),
  d_changedAddressRange()
{}

void JointControl::setValue(ushort value)
{
  value = Math::clamp(value, MX28::MIN_VALUE, MX28::MAX_VALUE);
  if (d_value == value)
    return;
  d_value = value;
  d_degrees = MX28::value2Degs(value);
  d_changedAddressRange.expand(MX28::P_GOAL_POSITION_L);
  d_changedAddressRange.expand(MX28::P_GOAL_POSITION_H);
}

void JointControl::setDegrees(double degrees)
{
  degrees = Math::clamp(degrees, MX28::MIN_DEGS, MX28::MAX_DEGS);
  d_degrees = degrees;
  unsigned value = MX28::degs2Value(degrees);
  if (d_value == value)
    return;
  d_value = value;
  d_changedAddressRange.expand(MX28::P_GOAL_POSITION_L);
  d_changedAddressRange.expand(MX28::P_GOAL_POSITION_H);
}

void JointControl::setRadians(double radians) { setDegrees(Math::radToDeg(radians)); }

void JointControl::setPGain(uchar p) { if (d_pGain == p) return; d_pGain = p; d_changedAddressRange.expand(MX28::P_P_GAIN); }
void JointControl::setIGain(uchar i) { if (d_iGain == i) return; d_iGain = i; d_changedAddressRange.expand(MX28::P_I_GAIN); }
void JointControl::setDGain(uchar d) { if (d_dGain == d) return; d_dGain = d; d_changedAddressRange.expand(MX28::P_D_GAIN); }

void JointControl::setPidGains(uchar p, uchar i, uchar d) { setPGain(p); setIGain(i); setDGain(d); }
