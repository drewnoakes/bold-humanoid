#pragma once

#include <string>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../Config/config.hh"
#include "../Math/math.hh"

namespace bold
{
  typedef std::function<Eigen::Vector2d(Eigen::Vector3d const&)> Projector;

  class CameraModel
  {
  public:
    CameraModel(unsigned imageWidth, unsigned imageHeight, double focalLength, double rangeVerticalDegs, double rangeHorizontalDegs)
    : d_imageWidth(imageWidth),
      d_imageHeight(imageHeight),
//       d_focalLength(focalLength),
      d_rangeVerticalDegs(rangeVerticalDegs),
      d_rangeHorizontalDegs(rangeHorizontalDegs)
    {}

    CameraModel()
    {
      d_imageWidth = Config::getStaticValue<int>("camera.image-width");
      d_imageHeight = Config::getStaticValue<int>("camera.image-height");
//       d_focalLength = Config::getStaticValue<double>("camera.focal-length");
      d_rangeVerticalDegs = Config::getStaticValue<double>("camera.field-of-view.vertical-degrees");
      d_rangeHorizontalDegs = Config::getStaticValue<double>("camera.field-of-view.horizontal-degrees");
    }

    unsigned imageWidth() const { return d_imageWidth; }
    unsigned imageHeight() const { return d_imageHeight; }

    double focalLength() const { return  1.0 / tan(.5 * rangeHorizontalRads()); } // TODO: cache
    double rangeVerticalDegs() const { return d_rangeVerticalDegs; }
    double rangeVerticalRads() const { return Math::degToRad(d_rangeVerticalDegs); }
    double rangeHorizontalDegs() const { return d_rangeHorizontalDegs; }
    double rangeHorizontalRads() const { return Math::degToRad(d_rangeHorizontalDegs); }

    /** Gets the direction, in camera coordinates, of the specified pixel.
     * Returns a unit vector.
     */
    Eigen::Vector3d directionForPixel(Eigen::Vector2d const& pixel) const;

    Maybe<Eigen::Vector2d> pixelForDirection(Eigen::Vector3d const& direction) const;

    /** Gets a projection matrix
     *
     * This matrix projects from camera frame onto image frame, up to
     * a scaling factor, which is given in the z element of the
     * transformed vector. i.e to get the pixel coordinate p of a
     * point v with projection matrix T: p' = Tv, p = p'/p'_z.
     */
    Eigen::Affine3d getProjectionTransform() const
    {
      // TODO: cache
      double f = focalLength();
      Eigen::Affine3d c;
      c.matrix() <<
        f, 0, 0, 0,
        0, f, 0, 0,
        0, 0, 1, 0,
        0, 0, 1, 0;

      auto s = Eigen::Scaling((d_imageWidth - 1) / 2.0, (d_imageHeight - 1) / 2.0, 1.0);
      Eigen::Affine3d t;
      t.matrix() <<
        -1, 0, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1;

      return s * c * t;
    }

  private:
    unsigned d_imageWidth;
    unsigned d_imageHeight;
//     double d_focalLength;
    double d_rangeVerticalDegs;
    double d_rangeHorizontalDegs;
  };
}
