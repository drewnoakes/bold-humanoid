#include "bodystate.hh"

#include "../../BodyModel/bodymodel.hh"
#include "../../Config/config.hh"
#include "../OrientationState/orientationstate.hh"
#include "../State/state.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

void BodyState::initialise(shared_ptr<BodyModel const> const& bodyModel, array<double,23> const& angles)
{
  //
  // Build all Joint and Limb transforms
  //

  d_torso = make_shared<LimbPosition const>(bodyModel->getRoot(), Affine3d::Identity());

  list<shared_ptr<BodyPartPosition const>> partQueue;
  partQueue.push_back(d_torso);

  while (!partQueue.empty())
  {
    shared_ptr<BodyPartPosition const> part = partQueue.front();
    partQueue.pop_front();

    if (shared_ptr<LimbPosition const> limbPosition = dynamic_pointer_cast<LimbPosition const>(part))
    {
      //
      // LIMB: Determine transformation of all connected joints by applying  proper translation.
      //

      auto const& limb = limbPosition->getLimb();
      d_limbByName[limb->name] = limbPosition;

      // Loop over all joints extending outwards from this limb
      for (auto& joint : limb->joints)
      {
        // Transformation = that of limb plus translation to the joint

        double angle = angles[(uchar)joint->id];

        auto jointPosition = make_shared<JointPosition>(joint, limbPosition->getTransform() * Translation3d(joint->anchors.first), angle);
        partQueue.push_back(jointPosition);
      }
    }
    else
    {
      //
      // JOINT: Determine transformation of joint, apply rotation, then subsequent translation to child part.
      //

      shared_ptr<JointPosition const> jointPosition = dynamic_pointer_cast<JointPosition const>(part);

      auto joint = jointPosition->getJoint();
      d_jointById[(uchar)joint->id] = jointPosition;

      Affine3d childTransform
        = jointPosition->getTransform()
        * AngleAxisd(joint->rotationOrigin + jointPosition->getAngleRads(), joint->axis)
        * Translation3d(-joint->anchors.second);

      ASSERT(joint->childPart);

      if (shared_ptr<Joint const> childJoint = dynamic_pointer_cast<Joint const>(joint->childPart))
      {
        double childJointAngle = angles[(uchar)childJoint->id];
        auto childJointPosition = make_shared<JointPosition>(childJoint, childTransform, childJointAngle);
        partQueue.push_back(childJointPosition);
      }
      else if (shared_ptr<Limb const> childLimb = dynamic_pointer_cast<Limb const>(joint->childPart))
      {
        auto childLimbPosition = make_shared<LimbPosition>(childLimb, childTransform);
        partQueue.push_back(childLimbPosition);
      }
      else
      {
        log::error("BodyState::initialise") << "Unknown body part type with name=" << joint->childPart->name;
      }
    }
  }

  //
  // Other bits
  //

  auto const& leftFootLimb = getLimb("left-foot");
  auto const& rightFootLimb = getLimb("right-foot");
  auto const& camera = getLimb("camera");

  double zl = leftFootLimb->getTransform().translation().z();
  double zr = rightFootLimb->getTransform().translation().z();

  auto lowestFoot = zl < zr ? leftFootLimb : rightFootLimb;

  // TODO this is an approximation
  d_torsoHeight = -std::min(zl, zr);

  static Setting<bool>* useOrientation = Config::getSetting<bool>("spatialiser.use-orientation");

  Affine3d agentTorsoRot;
  auto const& orientation = State::get<OrientationState>();
  if (useOrientation->getValue() && orientation)
  {
    agentTorsoRot = orientation->withoutYaw();
  }
  else
  {
    // TODO this rotation includes any yaw from the leg which isn't wanted
    agentTorsoRot = lowestFoot->getTransform().inverse().rotation();
  }

  Affine3d const& torsoCameraTr = camera->getTransform();

  // This is a special transform that gives the position of the camera in
  // the agent's frame, taking any rotation of the torso into account
  // considering the orientation of the foot (which is assumed to be flat.)
  auto agentTorsoTr = Translation3d(0, 0, d_torsoHeight) * agentTorsoRot;
  d_agentCameraTr = agentTorsoTr * torsoCameraTr;

  // We use the inverse a lot, so calculate it once here.
  d_cameraAgentTr = d_agentCameraTr.inverse();
}
