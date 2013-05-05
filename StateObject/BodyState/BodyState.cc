#include "bodystate.ih"

BodyState::BodyState(double angles[])
: d_torso(),
  d_jointById(),
  d_limbByName()
{
  initBody(angles);
  updatePosture();
  d_torsoHeight = Lazy<double>([this]()
  {
    return std::make_shared<double>(
      std::max(
        this->getLimb("lFoot")->transform.inverse().translation().z(),
        this->getLimb("rFoot")->transform.inverse().translation().z()
      )
    );
  });

  // TODO determine stance foot
  Affine3d const& footTorsoTransform = getLimb("rFoot")->transform;

  d_torsoWorldRotation = footTorsoTransform.inverse().rotation();

  Affine3d const& cameraTorsoTransform = getLimb("camera")->transform;

  // This is a special transform that gives the position of the camera in
  // the agent's frame, taking any rotation of the torso into account
  // considering the orientation of the foot (which is assumed to be flat.)
  d_cameraAgentTransform = Translation3d(0,0,-footTorsoTransform.translation().z()) * d_torsoWorldRotation * cameraTorsoTransform;
}
