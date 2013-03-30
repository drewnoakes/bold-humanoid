#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2i const& pixel,
                                                     double const torsoHeight,
                                                     Affine3d const& cameraTorsoTransform) const
{
  // The torso and world use different orientations of the axes, so we include that into the transform here
  // TODO simplify this
  Affine3d cameraAgentTransform = cameraTorsoTransform
    * AngleAxisd(-M_PI/2, Vector3d::UnitX()) // rotate z-axis to point upwards
    * AngleAxisd(M_PI, Vector3d::UnitZ());   // rotate y-axis to point forwards, with x-axis to right

  Vector3d direction = cameraAgentTransform * d_cameraModel->directionForPixel(pixel);
  Vector3d position = cameraAgentTransform * Vector3d::Zero();

  return Math::intersectRayWithGroundPlane(position, direction, -torsoHeight);
}