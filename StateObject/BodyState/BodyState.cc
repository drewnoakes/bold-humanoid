#include "bodystate.ih"

#include "../MX28Snapshot/mx28snapshot.hh"

BodyState::BodyState(double angles[], ulong cycleNumber)
: d_torso(),
  d_jointById(),
  d_limbByName(),
  d_motionCycleNumber(cycleNumber)
{
  initialise(angles);
}

BodyState::BodyState(shared_ptr<HardwareState const> const& hardwareState, ulong cycleNumber)
: d_torso(),
  d_jointById(),
  d_limbByName(),
  d_motionCycleNumber(cycleNumber)
{
  double angles[(uchar)JointId::MAX + 2];
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    angles[jointId] = hardwareState->getMX28State(jointId).presentPosition;
  angles[(uchar)JointId::CAMERA_TILT] = Math::degToRad(Config::getValue<double>("camera.vertical-angle-degrees"));

  initialise(angles);
}
