%{
#include <CameraModel/cameramodel.hh>
%}

namespace bold
{
  class CameraModel
  {
  public:
    unsigned imageWidth() const;
    unsigned imageHeight() const;

    double focalLength() const;
    double rangeVerticalDegs() const;
    double rangeVerticalRads() const;
    double rangeHorizontalDegs() const;
    double rangeHorizontalRads() const;

    Eigen::Vector3d directionForPixel(Eigen::Vector2d const& pixel) const;

    // Todo: make wrapper that handles Maybe
    //Maybe<Eigen::Vector2d> pixelForDirection(Eigen::Vector3d const& direction) const;
  };
}
