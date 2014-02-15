#include "bodystate.ih"

#include "../../BodyControl/bodycontrol.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"

BodyState::BodyState(double angles[23], vector<int> positionValueDiffs, ulong cycleNumber)
: d_positionValueDiffs(positionValueDiffs),
  d_torso(),
  d_jointById(),
  d_limbByName(),
  d_motionCycleNumber(cycleNumber)
{
  initialise(angles);
}

BodyState::BodyState(shared_ptr<HardwareState const> const& hardwareState, shared_ptr<BodyControl> const& bodyControl, ulong cycleNumber)
: d_positionValueDiffs((uchar)JointId::MAX + 1, 0),
  d_torso(),
  d_jointById(),
  d_limbByName(),
  d_motionCycleNumber(cycleNumber)
{
  // Add two extra as:
  // - we don't use index 0
  // - we add an extra joint for the camera's tilt within the head.
  double angles[23];

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    auto const& joint = hardwareState->getMX28State(jointId);
    angles[jointId] = joint.presentPosition;
    d_positionValueDiffs[jointId] = (int)bodyControl->getJoint((JointId)jointId)->getValue() - joint.presentPositionValue;
  }

  static auto tiltSetting = Config::getSetting<double>("camera.calibration.tilt-angle-degrees");
  static auto panSetting = Config::getSetting<double>("camera.calibration.pan-angle-degrees");

  // Set the camera head tilt according to the configured angle
  angles[(uchar)JointId::CAMERA_CALIB_TILT] = Math::degToRad(tiltSetting->getValue());
  angles[(uchar)JointId::CAMERA_CALIB_PAN ] = Math::degToRad(panSetting->getValue());

  initialise(angles);
}
