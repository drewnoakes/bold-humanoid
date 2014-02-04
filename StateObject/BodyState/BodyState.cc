#include "bodystate.ih"

#include "../../MX28Snapshot/mx28snapshot.hh"

BodyState::BodyState(double angles[], ulong cycleNumber)
: d_torso(),
  d_jointById(),
  d_limbByName(),
  d_motionCycleNumber(cycleNumber),
  d_cameraTilt(Config::getSetting<double>("camera.vertical-angle-degrees"))
{
  initialise(angles);
}

BodyState::BodyState(shared_ptr<HardwareState const> const& hardwareState, ulong cycleNumber)
: d_torso(),
  d_jointById(),
  d_limbByName(),
  d_motionCycleNumber(cycleNumber),
  d_cameraTilt(Config::getSetting<double>("camera.vertical-angle-degrees"))
{
  // Add two extra as:
  // - we don't use index 0
  // - we add an extra joint for the camera's tilt within the head.
  double angles[(uchar)JointId::MAX + 2];

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    angles[jointId] = hardwareState->getMX28State(jointId).presentPosition;

  // Set the camera head tilt according to the configured angle
  angles[(uchar)JointId::CAMERA_TILT] = Math::degToRad(d_cameraTilt->getValue());

  initialise(angles);
}
