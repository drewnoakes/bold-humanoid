#ifndef BOLD_MATH_HH
#define BOLD_MATH_HH

#include <Eigen/Core>

#include "../util/Maybe.hh"

namespace bold
{
  class Math
  {
  public:
    static Maybe<Eigen::Vector3d> intersectRayWithGroundPlane(Eigen::Vector3d const& position,
                                                              Eigen::Vector3d const& direction,
                                                              double const planeZ);

    static Maybe<Eigen::Vector3d> intersectRayWithPlane(Eigen::Vector3d const& position,
                                                        Eigen::Vector3d const& direction,
                                                        Eigen::Vector4d const& plane);

  private:
    Math() {}
  };
}

#endif