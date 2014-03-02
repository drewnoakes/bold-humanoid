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
    /// Initialises a CameraModel using parameters defined in configuration.
    CameraModel()
    : CameraModel(Config::getStaticValue<int>("camera.image-width"),
                  Config::getStaticValue<int>("camera.image-height"),
                  Config::getStaticValue<double>("camera.field-of-view.vertical-degrees"),
                  Config::getStaticValue<double>("camera.field-of-view.horizontal-degrees"))
    {}

    /// Initialises a CameraModel using the specified parameters.
    CameraModel(ushort imageWidth, ushort imageHeight, double rangeVerticalDegs, double rangeHorizontalDegs);

    ushort imageWidth() const { return d_imageWidth; }
    ushort imageHeight() const { return d_imageHeight; }

    double focalLength() const { return d_focalLength; }
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
    Eigen::Affine3d getProjectionTransform() const { return d_projectionTransform; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  private:
    ushort d_imageWidth;
    ushort d_imageHeight;
    double d_focalLength;
    double d_rangeVerticalDegs;
    double d_rangeHorizontalDegs;
    Eigen::Affine3d d_projectionTransform;
  };
}
