#ifndef BOLD_MATH_HH
#define BOLD_MATH_HH

#include <Eigen/Core>
#include <functional>

#include "../geometry/LineSegment.hh"
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

    static Eigen::Vector2d linePointClosestToPoint(LineSegment2d const& segment,
                                                   Eigen::Vector2d const& point);

    // TODO what if 'vector' has zero length? should this return 'Maybe<Vector2d>'?
    static Eigen::Vector2d findPerpendicularVector(Eigen::Vector2d const& vector);

    static std::function<double()> createUniformRng(double min, double max, bool randomSeed = true);
    static std::function<double()> createNormalRng(double mean, double stddev, bool randomSeed = true);

    static double degToRad(double degrees) { return (degrees * M_PI) / 180.0; }
    static double radToDeg(double radians) { return (radians / M_PI) * 180.0; }

    static double smallestAngleBetween(Eigen::Vector2d v1, Eigen::Vector2d v2);

  private:
    Math() {}
  };
}

#endif