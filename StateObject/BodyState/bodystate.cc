#include "bodystate.hh"

#include "../HardwareState/hardwarestate.hh"
#include "../../BodyControl/bodycontrol.hh"
#include "../../BodyModel/bodymodel.hh"
#include "../../Config/config.hh"
#include "../../MX28/mx28.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

LimbPosition::LimbPosition(shared_ptr<Limb const> limb, Affine3d const& transform)
  : BodyPartPosition(transform),
    d_limb(limb)
{
  ASSERT(limb);
}

Vector3d LimbPosition::getCentreOfMassPosition() const
{
  return getTransform() * getLimb()->com;
}

////////////////////////////////////////////////////////////////////////////////

JointPosition::JointPosition(shared_ptr<Joint const> joint, Affine3d const& transform, double angleRads)
  : BodyPartPosition(transform),
    d_joint(joint),
    d_angleRads(angleRads)
{
  ASSERT(joint);
}

////////////////////////////////////////////////////////////////////////////////

BodyState::BodyState(shared_ptr<BodyModel const> const& bodyModel, array<double,23> const& angles, array<short,21> const& positionValueDiffs, ulong cycleNumber)
: d_positionValueDiffById(positionValueDiffs),
  d_torso(),
  d_jointById(),
  d_limbByName(),
  d_isCentreOfMassComputed(false),
  d_motionCycleNumber(cycleNumber)
{
  initialise(bodyModel, angles);
}

BodyState::BodyState(shared_ptr<BodyModel const> const& bodyModel, shared_ptr<HardwareState const> const& hardwareState, shared_ptr<BodyControl> const& bodyControl, ulong cycleNumber)
: d_torso(),
  d_jointById(),
  d_limbByName(),
  d_isCentreOfMassComputed(false),
  d_motionCycleNumber(cycleNumber)
{
  // Add three extra as:
  // - we don't use index 0
  // - we add two extra joints for the camera's calibration pan & tilt within the head
  array<double,23> angles;
  angles[0] = 0;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    auto const& joint = hardwareState->getMX28State(jointId);
    JointControl* jointControl = bodyControl->getJoint((JointId)jointId);

    angles[jointId] = joint.presentPosition;
    d_positionValueDiffById[jointId] = (short)jointControl->getValue() + jointControl->getModulationOffset() - joint.presentPositionValue;
  }

  static auto tiltSetting = Config::getSetting<double>("camera.calibration.tilt-angle-degrees");
  static auto panSetting = Config::getSetting<double>("camera.calibration.pan-angle-degrees");

  // Set the camera head tilt according to the configured angle
  angles[(uchar)JointId::CAMERA_CALIB_TILT] = Math::degToRad(tiltSetting->getValue());
  angles[(uchar)JointId::CAMERA_CALIB_PAN ] = Math::degToRad(panSetting->getValue());

  initialise(bodyModel, angles);
}

Vector3d const& BodyState::getCentreOfMass() const
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

shared_ptr<LimbPosition const> BodyState::getLimb(string const& name) const
{
  // NOTE cannot use '[]' on a const map
  auto const& i = d_limbByName.find(name);
  if (i == d_limbByName.end())
  {
    log::error("BodyState::getJoint") << "Invalid limb name: " << name;
    throw runtime_error("Invalid limb name: " + name);
  }
  return i->second;
}

shared_ptr<JointPosition const> BodyState::getJoint(JointId jointId) const
{
  ASSERT(jointId >= JointId::MIN && jointId <= JointId::MAX);
  return d_jointById[(uchar)jointId];
}

void BodyState::visitJoints(function<void(shared_ptr<JointPosition const> const&)> visitor) const
{
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    visitor(d_jointById[(uchar)jointId]);
}

void BodyState::visitLimbs(function<void(shared_ptr<LimbPosition const> const&)> visitor) const
{
  for (auto const& pair : d_limbByName)
    visitor(pair.second);
}

Affine3d BodyState::determineFootAgentTr(bool leftFoot) const
{
  auto footTorsoTr = getLimb(leftFoot ? "left-foot" : "right-foot")->getTransform().inverse();

  return Math::alignUp(footTorsoTr);
}

shared_ptr<BodyState const> BodyState::zero(shared_ptr<BodyModel const> const& bodyModel, ulong thinkCycleNumber)
{
  array<double,23> angles;
  angles.fill(0);

  // Tilt the head up slightly, so that we can see the horizon in the image (better for testing)
  angles[(int)JointId::HEAD_TILT] = MX28::degs2Value(20.0);

  array<short,21> positionValueDiffs;
  positionValueDiffs.fill(0);

  return make_shared<BodyState>(bodyModel, angles, positionValueDiffs, thinkCycleNumber);
}
