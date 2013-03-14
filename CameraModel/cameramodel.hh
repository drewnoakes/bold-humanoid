#ifndef BOLD_CAMERA_MODEL_HH
#define BOLD_CAMERA_MODEL_HH

#include <string>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace bold
{
  typedef std::function<Eigen::Vector2i(Eigen::Vector3d const&)> Projector;

  class CameraModel
  {
  public:
    CameraModel(unsigned imageWidth, unsigned imageHeight, double focalLength, double rangeVertical, double rangeHorizontal)
    : d_imageWidth(imageWidth),
      d_imageHeight(imageHeight),
      d_focalLength(focalLength),
      d_rangeVertical(rangeVertical),
      d_rangeHorizontal(rangeHorizontal)
    {}

    unsigned imageWidth() const { return d_imageWidth; }
    unsigned imageHeight() const { return d_imageHeight; }
    double focalLength() const { return d_focalLength; }
    double rangeVertical() const { return d_rangeVertical; }
    double rangeHorizontal() const { return d_rangeHorizontal; }

    Eigen::Affine3d getProjectionTransform() const
    {
      Eigen::Matrix4d mat;

      mat <<
        d_focalLength, 0, 0, 0,
        0, d_focalLength, 0, 0,
        0, 0, d_focalLength, 0,
        0, 0, 1, 0;

      return Eigen::Affine3d(mat);
    }

    /** Gets a projector to convert from 3D camera space (camera at 0,0,0
     * looking down +ve z) to 2D screen space (with camera in centre of image.)
     */
    Projector getProjector() const
    {
      auto mat = getProjectionTransform();
      double projectionPlaneWidth = atan(d_rangeHorizontal / 2) * d_focalLength * 2;
      double pixelWidth = projectionPlaneWidth / d_imageWidth;
      return [mat,pixelWidth](Eigen::Vector3d const& in) {
        auto m = (mat * in);
        return (m.head<2>() / pixelWidth).cast<int>();
      };
    }

  private:
    unsigned d_imageWidth;
    unsigned d_imageHeight;
    double d_focalLength;
    double d_rangeVertical;
    double d_rangeHorizontal;
  };
}

#endif
