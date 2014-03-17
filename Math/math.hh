#pragma once

#include <Eigen/Core>
#include <functional>

#include "../geometry/LineSegment/linesegment.hh"
#include "../geometry/LineSegment/LineSegment2/linesegment2.hh"
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

    template<typename T>
    static double clamp(T val, T min, T max)
    {
      if (val < min)
        return min;
      if (val > max)
        return max;
      return val;
    }

    /**
      * Maps @link input to a value in the range from @link lowerOutput to @link upperOutput.
      * If @link input is outside the range from @link lower to @link upper, then @link lowerOutput or @link upperOutput are returned,
      * otherwise the value is linearly interpolated.
      * Note that @link lower must be less than @link upper, but that there is no restriction on values of @link lowerOutput and @link upperOutput.
      */
    template<typename T>
    static T lerp(double const& input, double const& lower, double const& upper, T const& lowerOutput, T const& upperOutput)
    {
      if (upper <= lower)
        throw std::runtime_error("lower must be less than upper");

      double ratio = (input - lower) / (upper - lower);
      ratio = clamp(ratio, 0.0, 1.0);

      return lowerOutput + (upperOutput - lowerOutput) * ratio;
    };

    template<typename T>
    static T lerp(double const& ratio, T const& lowerOutput, T const& upperOutput)
    {
      return lowerOutput + (upperOutput - lowerOutput) * ratio;
    };

  private:
    Math() {}
  };
}
