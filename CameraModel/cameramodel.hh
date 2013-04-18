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
      // TODO have seen both 58.0 and 60.0 as default horizontal range values
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

    Eigen::Vector2i pixelForDirection(Eigen::Vector3d const& direction) const;

    /** Gets a projection matrix
     *
     * This matrixt projects from camera frame onto image frame, up to
     * a scaling factor, which is given in the z element of the
     * transformed vector. i.e to get the pixel coordinate p of a
     * point v with projection matrix T: p' = Tv, p = p'/p'_z.
     */
    Eigen::Affine3d getProjectionTransform() const
    {
      // TODO: cache
      double f = 1.0 / tan(.5 * rangeHorizontalRads());
      Eigen::Affine3d c;
      c.matrix() <<
        f, 0, 0, 0,
        0, f, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
      
      auto s = Eigen::Scaling((d_imageWidth - 1) / 2.0, (d_imageHeight - 1) / 2.0, 1.0);
      Eigen::Affine3d t;
      t.matrix() <<
        -1, 0, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1;
      
      return s * c * t;
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
        return ((m / m.z()).head<2>() / pixelWidth).cast<int>();
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
