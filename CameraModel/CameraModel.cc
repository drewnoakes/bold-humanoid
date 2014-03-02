#include "cameramodel.ih"

/// Initialises a CameraModel using the specified parameters.
CameraModel::CameraModel(ushort imageWidth, ushort imageHeight, double rangeVerticalDegs, double rangeHorizontalDegs)
  : d_imageWidth(imageWidth),
    d_imageHeight(imageHeight),
    d_rangeVerticalDegs(rangeVerticalDegs),
    d_rangeHorizontalDegs(rangeHorizontalDegs)
{
  //
  // Compute the focal length
  //
  d_focalLength = 1.0 / tan(.5 * rangeHorizontalRads());

  //
  // Compute the projection transform
  //

  // Perspective transform
  double f = focalLength();
  Eigen::Affine3d c;
  c.matrix() <<
    f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, 1, 0,
    0, 0, 1, 0;

  double r = tan(0.5 * rangeVerticalRads()) / tan(0.5 * rangeHorizontalRads());

  double ws = (d_imageWidth - 1.0) / 2.0;
  double hs = (d_imageHeight - 1.0) / 2.0;
  // c maps vof to x/y in [-1,1]; scale to pixel measurements and transform to have origin in corner pixel
  auto s =
    Eigen::Translation3d(d_imageWidth / 2.0, d_imageHeight / 2.0, 0) * 
    Eigen::Scaling(ws, hs, 1.0) *
    Eigen::Scaling(1.0, 1.0 / r, 1.0);

  // agent frame: x right, y forward, z up
  // camera frame: x left, y up, z forward (remember: camera is upside down)
  Eigen::Affine3d t;
  t.matrix() <<
    -1, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1;

  // Full transform from agent frame to image
  d_projectionTransform = s * c * t;
}
