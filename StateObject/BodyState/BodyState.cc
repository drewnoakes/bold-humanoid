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

  Eigen::Affine3d const& cameraTransform = getLimb("camera")->transform;
  Eigen::Affine3d const& footTransform = getLimb("rFoot")->transform;
  d_cameraGroundTransform = footTransform.inverse() * cameraTransform;
}
