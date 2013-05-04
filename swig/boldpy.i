%module bold
%{
#include "../Math/math.hh"
%}

namespace bold
{
  class Math
  {
  public:
    static double smallestAngleBetween(Eigen::Vector2d v1, Eigen::Vector2d v2);
    static double degToRad(double degrees);
  private:
    Math() {}
  };
}
