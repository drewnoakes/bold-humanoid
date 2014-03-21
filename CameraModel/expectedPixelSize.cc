#include "cameramodel.ih"

unsigned CameraModel::expectedPixelSize(Vector3d const& position, double diameter)
{
  // Up in camera frame
  Vector3d up(0, 0, 1);
  
  // Perpendicular to object direction
  Vector3d perp = position.cross(up).normalized();

  double s = (getProjectionTransform() * (diameter * perp)).norm();
  cout << "s: " << s << endl;
  return (s + 0.5);
}

