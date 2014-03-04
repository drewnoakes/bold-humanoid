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
  Eigen::Affine3d c;
  c.matrix() <<
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 1, 0;

  double ws = d_imageWidth / 2.0;
  double hs = d_imageHeight / 2.0;
  // c maps vof to x/y in [-1,1]; scale to pixel measurements and transform to have origin in corner pixel
  auto s =
    //Eigen::Translation3d(d_imageWidth / 2.0, d_imageHeight / 2.0, 0) * 
    Eigen::Scaling(ws / tan(.5 * rangeHorizontalRads()),
                   hs / tan(.5 * rangeVerticalRads()),
                   1.0);

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
