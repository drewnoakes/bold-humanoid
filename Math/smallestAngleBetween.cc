#include "math.ih"

double Math::smallestAngleBetween(Vector2d v1, Vector2d v2)
{
  auto normalisedDot = v1.normalized().dot(v2.normalized());
  return acos(normalisedDot);
}
