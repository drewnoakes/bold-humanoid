#include "bodystate.ih"

#include "../../BodyControl/bodycontrol.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"

BodyState::BodyState(shared_ptr<BodyModel const> const& bodyModel, array<double,23> const& angles, array<short,21> const& positionValueDiffs, ulong cycleNumber)
: d_positionValueDiffs(positionValueDiffs),
  d_torso(),
  d_jointById(),
  d_limbByName(),
  d_motionCycleNumber(cycleNumber)
{
  initialise(bodyModel, angles);
}

BodyState::BodyState(shared_ptr<BodyModel const> const& bodyModel, shared_ptr<HardwareState const> const& hardwareState, shared_ptr<BodyControl> const& bodyControl, ulong cycleNumber)
: d_torso(),
  d_jointById(),
  d_limbByName(),
  d_motionCycleNumber(cycleNumber)
{
  // Add three extra as:
  // - we don't use index 0
  // - we add two extra joints for the camera's pan & tilt within the head.
  array<double,23> angles;
  angles[0] = 0;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    auto const& joint = hardwareState->getMX28State(jointId);
    angles[jointId] = joint.presentPosition;
    d_positionValueDiffs[jointId] = (short)bodyControl->getJoint((JointId)jointId)->getValue() - joint.presentPositionValue;
  }

  static auto tiltSetting = Config::getSetting<double>("camera.calibration.tilt-angle-degrees");
  static auto panSetting = Config::getSetting<double>("camera.calibration.pan-angle-degrees");

  // Set the camera head tilt according to the configured angle
  angles[(uchar)JointId::CAMERA_CALIB_TILT] = Math::degToRad(tiltSetting->getValue());
  angles[(uchar)JointId::CAMERA_CALIB_PAN ] = Math::degToRad(panSetting->getValue());

  initialise(bodyModel, angles);
}

Eigen::Vector3d const& BodyState::getCentreOfMass() const
{
  if (!d_isCentreOfMassComputed)
  {
    double totalMass = 0.0;
    Vector3d weightedSum(0,0,0);

    visitLimbs([&](shared_ptr<LimbPosition const> limbPosition)
    {
      double mass = limbPosition->getLimb()->mass;
      totalMass += mass;
      weightedSum += mass * limbPosition->getCentreOfMassPosition();
    });

    d_centreOfMass = weightedSum / totalMass;
    d_isCentreOfMassComputed = true;
  }

  return d_centreOfMass;
}

Eigen::Vector3d LimbPosition::getCentreOfMassPosition() const
{
  return getTransform() * getLimb()->com;
}
