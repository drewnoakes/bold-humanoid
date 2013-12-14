#include "spatialiser.ih"

int Spatialiser::findHorizonForColumn(int column)
{
  auto body = AgentState::get<BodyState>();

  Affine3d const& cameraAgentTransform = body->getCameraAgentTransform();

  return findHorizonForColumn(column, cameraAgentTransform);
}

int Spatialiser::findHorizonForColumn(int column, Affine3d const& cameraGroundTransform)
{
  // Equation of horizon line:
  // http://mi.eng.cam.ac.uk/~cipolla/lectures/4F12/Examples/old/solutions2.pdf
  //
  // | x |
  // | y | . n = 0
  // | f |
  //
  // Where n is normal of plane parallel to ground (in image frame).
  //
  // x nx + f ny + z nz = 0
  // y = -1/ny (x nx + f nz)

  assert(d_cameraModel);
  assert(d_cameraModel->imageWidth() > 1);

  // x on projection plane
  double x = (2.0 * column / (d_cameraModel->imageWidth() - 1.0)) - 1.0;

  // From camera to image frame
  // TODO: cache
  Eigen::Affine3d t;
  t.matrix() <<
    -1, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1;

  auto torsoToImage = t * cameraGroundTransform.inverse();

  // Normal to ground plane
  Vector3d up = torsoToImage.matrix().col(2).head<3>();

  // In this case, we're looking straight up or down
  // TODO: handle better
  assert(up.y() != 0);

  double f = d_cameraModel->focalLength();

  double y = -1 / up.y() * ( x * up.x() + f * up.z());

  double r = f * tan(d_cameraModel->rangeVerticalRads() / 2);

  int py = ((y / r + 1.0) * (d_cameraModel->imageHeight() - 1.0) / 2.0) + .5;
  return py;
}
