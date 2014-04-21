#include "spatialiser.ih"

int Spatialiser::findHorizonForColumn(int column)
{
  return findHorizonForColumn(column, State::get<BodyState>(StateTime::CameraImage)->getCameraAgentTransform());
}

int Spatialiser::findHorizonForColumn(int column, Affine3d const& cameraAgentTr)
{
  // TODO: can we derive directly from perspective transform?

  // 
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

  ASSERT(d_cameraModel);
  ASSERT(d_cameraModel->imageWidth() > 1);

  // x on projection plane
  // Todo: should be column + 0.5 to match convention elsewhere?
  double x = (2.0 * column / (d_cameraModel->imageWidth() - 1.0)) - 1.0;

  // From camera to clip space frame (intermediate of camera to image transform)
  // TODO: cache
  Eigen::Affine3d clipCameraTr;
  clipCameraTr.matrix() <<
    -1, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1;

  auto clipAgentTr = clipCameraTr * cameraAgentTr;

  // Normal to ground plane, in clip frame
  Vector3d up = clipAgentTr.matrix().col(2).head<3>();

  // In this case, we're looking straight up or down
  // TODO: handle better
  ASSERT(up.y() != 0);

  double f = d_cameraModel->focalLength();

  // Solution to | x y f |^T . n = 0
  double y = -1 / up.y() * ( x * up.x() + f * up.z());

  double r = f * tan(d_cameraModel->rangeVerticalRads() / 2);

  // TODO: check if matches convention
  int py = ((y / r + 1.0) * (d_cameraModel->imageHeight() - 1.0) / 2.0) + .5;
  return py;
}
