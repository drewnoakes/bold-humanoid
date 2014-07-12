#pragma once

#include "../Math/math.hh"

#include <Eigen/Core>

namespace bold
{
/*
  enum class PointType
  {
    /// The point is on the lower bound of an occlusion.
    /// In 2D, this is the bottom of the image.
    /// In 3D this is the nearest point in the occluded area along a ray from the viewer.
    Occlusion = 1,
    /// The point is visually at the top of an area considered as the field, although this may be the camera edge
    FieldEdge = 2,
    /// Point is located at the boundary of the camera image
    CameraEdge = 3
  };
*/

  template<typename T,int Dim=2>
  class OcclusionRay
  {
  public:
    typedef Eigen::Matrix<T,Dim,1> Point;

    OcclusionRay(Point near, Point far/*, PointType nearType, PointType farType*/)
    : d_near(near),
      d_far(far)/*,
      d_nearType(nearType),
      d_farType(farType)*/
    {}

    Point const& near() const { return d_near; }
    Point const& far() const { return d_far; }
//    PointType const& nearType() const { return d_nearType; }
//    PointType const& farType() const { return d_farType; }

    double constexpr norm() const { return (d_near - d_far).norm(); }
    double constexpr angle() const { return Math::angleToPoint(d_near); }

//    double distance() const;
//    bool isOpenField() const;
//    bool isOcclusion() const;
//    bool isFieldEdge() const;

  private:
    Point d_near;
    Point d_far;
//    PointType d_nearType;
//    PointType d_farType;
  };
}
