#pragma once

#include <vector>
#include <Eigen/Core>

namespace bold
{
  enum class HalfHull
  {
    Top,
    Bottom
  };

  // TODO is it better to specify the side as a template parameter to compile away conditions?
  // TODO below doesn't seem to work with ushort... is the problem being unsigned, or integral?

  template<typename T>
  class HalfHullBuilder
  {
    typedef Eigen::Matrix<T,2,1> Vector2;

  public:
    /** Calculates half of a convex hull along a line of points.
     *
     * Inputs are assumed to be ordered in increasing x direction, with no duplicate x values.
     */
    std::vector<Vector2> findHalfHull(std::vector<Vector2> const& input, HalfHull side)
    {
      int n = input.size(),
          k = 0;

      std::vector<Vector2> hull(n);

      if (side == HalfHull::Bottom)
      {
        for (int i = 0; i < n; i++)
        {
          while (k >= 2 && cross(hull[k-2], hull[k-1], input[i]) <= 0)
            k--;

          hull[k++] = input[i];
        }
      }
      else
      {
        for (int i = 0; i < n; i++)
        {
          while (k >= 2 && cross(hull[k-2], hull[k-1], input[i]) >= 0)
            k--;

          hull[k++] = input[i];
        }
      }

      hull.resize(k);

      return hull;
    }

  private:
    static T cross(Vector2 const& O, Vector2 const& A, Vector2 const& B)
    {
      return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
    }
  };
}
