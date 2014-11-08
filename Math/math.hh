#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <functional>

#include "../geometry/LineSegment/linesegment.hh"
#include "../geometry/LineSegment/LineSegment2/linesegment2.hh"
#include "../util/assert.hh"
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

    static constexpr double degToRad(double degrees) { return (degrees * M_PI) / 180.0; }
    static constexpr double radToDeg(double radians) { return (radians / M_PI) * 180.0; }

    static double smallestAngleBetween(Eigen::Vector2d v1, Eigen::Vector2d v2);

    static Eigen::Affine3d alignUp(Eigen::Affine3d const& transform);

    template<typename T>
    static constexpr T clamp(T val, T min, T max)
    {
      return val < min
        ? min
        : val > max
          ? max
          : val;
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
      if (unlikely(upper <= lower))
        throw std::runtime_error("lower must be less than upper");

      double ratio = (input - lower) / (upper - lower);
      ratio = clamp(ratio, 0.0, 1.0);

      return lowerOutput + (upperOutput - lowerOutput) * ratio;
    }

    template<typename T>
    static constexpr T lerp(double const& ratio, T const& lowerOutput, T const& upperOutput)
    {
      return lowerOutput + (upperOutput - lowerOutput) * ratio;
    }

    /** Constrains the angle to range [-PI,PI). */
    static double normaliseRads(double rads)
    {
      rads = fmod(rads + M_PI, 2*M_PI);
      if (rads < 0)
          rads += 2*M_PI;
      return rads - M_PI;
    }

    /** Absolute distance between two angles in radians
     *
     * e.g.:
     * |pi - .5 pi| = |.5 pi - pi| = .5 pi
     * | -.1 po - .1pi | = | .1pi - -.1 pi | = .2 pi
     * | -.9 pi - .9 pi | = | .9 pi - -.9pi | = .2 pi
     */
    static double shortestAngleDiffRads(double a1, double a2)
    {
      // The fmod() function computes the floating-point remainder of
      // dividing x by y.  The return value is x - n * y, where n is
      // the quotient of x / y, rounded toward zero to an integer.
      double d = fmod(a2 - a1, 2*M_PI);

      d += (d > M_PI)
        ? -2*M_PI
        : d <= -M_PI
          ? 2*M_PI
          : 0;

      return d;
    }

    /** Returns the angle to a point, as defined in the agent frame,
     * where zero is straight ahead and positive is to the left
     * (counter-clockwise). */
    template<int N>
    static double angleToPoint(Eigen::Matrix<double, N, 1> const& point)
    {
      static_assert(N > 1, "Vector must have at least two dimensions");
      return ::atan2(-point.x(), point.y());
    }

    /** Returns the point at the given angle and distance, as defined
     * in the agent frame, where zero is straight ahead and positive
     * is to the left (counter-clockwise). */
    static inline Eigen::Vector2d pointAtAngle(double angle, double distance)
    {
      return Eigen::Vector2d(cos(angle) * distance, sin(angle) * distance);
    }

  private:
    Math() = delete;
  };
}
