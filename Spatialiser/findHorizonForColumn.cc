#include "spatialiser.ih"

int Spatialiser::findHorizonForColumn(int column, Affine3d const& cameraTorsoTransform)
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
  
  auto torsoToImage = t * cameraTorsoTransform.inverse();

  // Normal to ground plane
  Vector3d up = torsoToImage.matrix().col(2).head<3>();

  // In this case, we're looking straight up or down
  // TODO: handle better
  assert(up.z() != 0);

  double f = d_cameraModel->focalLength();

  // Not 100% sure why we need to multiple x with f as well
  double y = -1 / up.y() * (f * x * up.x() + f * up.z()); 

  int py = (y / d_cameraModel->focalLength() + 1.0) * d_cameraModel->imageHeight() / 2;
  return py;
}
