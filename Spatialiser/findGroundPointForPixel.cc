#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, double groundZ) const
{
  return findGroundPointForPixel(pixel, d_zeroGroundPixelTr);
}

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel,
                                                     Affine3d const& agentCameraTr,
                                                     double groundZ) const
{
  auto groundPixelTr = findGroundPixelTransform(agentCameraTr, groundZ);
  return findGroundPointForPixel(pixel, groundPixelTr);
}

Maybe<Eigen::Vector3d> Spatialiser::findGroundPointForPixel(Eigen::Vector2d const& pixel, Eigen::Matrix3d const& groundPixelTr) const
{
  auto b = Vector3d{pixel(0) - d_cameraModel->imageWidth() / 2.0,
                    pixel(1) - d_cameraModel->imageHeight() / 2.0,
                    1};

  auto pt = groundPixelTr * b;

  if (pt(2) < 0)
    return Maybe<Vector3d>::empty();
  else
  {
    auto point = Vector3d{pt(0) / pt(2), pt(1) / pt(2), 0.0};
    return Maybe<Vector3d>{point};
  }
}

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint) const
{
  return findPixelForAgentPoint(agentPoint, State::get<BodyState>(StateTime::CameraImage)->getAgentCameraTransform());
}

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint, Affine3d const& agentCameraTransform) const
{
  return d_cameraModel->pixelForDirection(agentCameraTransform * agentPoint);
}
