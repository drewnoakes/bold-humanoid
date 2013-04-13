#ifndef BOLD_CAMERA_MODEL_HH
#define BOLD_CAMERA_MODEL_HH

#include <string>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <minIni.h>

namespace bold
{
  typedef std::function<Eigen::Vector2i(Eigen::Vector3d const&)> Projector;

  class CameraModel
  {
  public:
    CameraModel(unsigned imageWidth, unsigned imageHeight, double focalLength, double rangeVerticalDegs, double rangeHorizontalDegs)
    : d_imageWidth(imageWidth),
      d_imageHeight(imageHeight),
      d_focalLength(focalLength),
      d_rangeVerticalDegs(rangeVerticalDegs),
      d_rangeHorizontalDegs(rangeHorizontalDegs)
    {}

    CameraModel(minIni const& ini)
    {
      d_imageWidth = ini.geti("Camera", "ImageWidth", 320);
      d_imageHeight = ini.geti("Camera", "ImageHeight", 240);
      d_focalLength = ini.getd("Camera", "FocalLength", 0.025);
      d_rangeVerticalDegs = ini.getd("Camera", "RangeVerticalDegrees", 46.0);
      d_rangeHorizontalDegs = ini.getd("Camera", "RangeHorizontalDegrees", 58.0);
    }

    unsigned imageWidth() const { return d_imageWidth; }
    unsigned imageHeight() const { return d_imageHeight; }
    double focalLength() const { return d_focalLength; }
    double rangeVerticalDegs() const { return d_rangeVerticalDegs; }
    double rangeVerticalRads() const { return d_rangeVerticalDegs/180.0 * M_PI; }
    double rangeHorizontalDegs() const { return d_rangeHorizontalDegs; }
    double rangeHorizontalRads() const { return d_rangeHorizontalDegs/180.0 * M_PI; }

    /** Gets the direction, in camera coordinates, of the specified pixel.
     * Returns a unit vector.
     */
    Eigen::Vector3d directionForPixel(Eigen::Vector2i const& pixel) const;

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
      double projectionPlaneWidth = atan(rangeHorizontalRads() / 2) * d_focalLength * 2;
      double pixelWidth = projectionPlaneWidth / d_imageWidth;
      return [mat,pixelWidth](Eigen::Vector3d const& in) -> Eigen::Vector2i {
        auto m = (mat * in);
        return (m.head<2>() / pixelWidth).cast<int>();
      };
    }

  private:
    unsigned d_imageWidth;
    unsigned d_imageHeight;
    double d_focalLength;
    double d_rangeVerticalDegs;
    double d_rangeHorizontalDegs;
  };
}

#endif
