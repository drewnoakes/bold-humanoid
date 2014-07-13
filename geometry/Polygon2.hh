#pragma once

#include <vector>
#include <type_traits>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include "Bounds.hh"
#include "LineSegment/linesegment.hh"
#include "LineSegment/LineSegment2/linesegment2.hh"
#include "../../util/assert.hh"
#include "../../util/log.hh"
#include "../../util/Maybe.hh"

namespace bold
{
  typedef unsigned int uint;

  template<typename T>
  class Polygon2
  {
  public:
    typedef Eigen::Matrix<T, 2, 1> Point;
    typedef std::vector<Point,Eigen::aligned_allocator<Point>> PointVector;

    Polygon2(PointVector const& vertices)
    : d_vertices(vertices)
    {
      if (vertices.size() < 3)
      {
        log::error("Polygon2::Polygon2") << "Cannot create a polygon with number of vertices: " << vertices.size();
        throw std::runtime_error("A polygon must have at least three vertices");
      }
    }

    Polygon2(Bounds<T,2> bounds)
    : d_vertices()
    {
      d_vertices.push_back(bounds.min());
      d_vertices.emplace_back(Point(bounds.max().x(), bounds.min().y()));
      d_vertices.push_back(bounds.max());
      d_vertices.emplace_back(Point(bounds.min().x(), bounds.max().y()));
    }

    bool contains(Point const& point)
    {
      bool isInside = false;
      for (int i = 0, j = d_vertices.size() - 1; i < d_vertices.size(); j = i++)
      {
        Point const& a = d_vertices[i];
        Point const& b = d_vertices[j];

        if (((a.y() > point.y()) != (b.y() > point.y()))
          && (point.x() < (b.x() - a.x()) * (point.y() - a.y()) / (b.y() - a.y()) + a.x()))
        {
          isInside = !isInside;
        }
      }
      return isInside;
    }

    uint vertexCount() const { return d_vertices.size(); }

    Point operator[](uint i)
    {
      return d_vertices[i];
    }

    typename PointVector::iterator begin() { return d_vertices.begin(); }
    typename PointVector::iterator end() { return d_vertices.end(); }
    typename PointVector::const_iterator begin() const { return d_vertices.begin(); }
    typename PointVector::const_iterator end() const { return d_vertices.end(); }

    /// Returns the subsection of the provided that that resides within this polygon.
    /// Assumes the poly is convex. Returns an empty result if no intersection exists.
    Maybe<LineSegment2<T>> clipLine(LineSegment2<T> const& line)
    {
      bool contains1 = contains(line.p1());
      bool contains2 = contains(line.p2());

      // Line is completely inside the polygon.
      // As we're convex, there cannot be any intersection.
      if (contains1 && contains2)
        return line;

      PointVector intersectionPoints;
      for (int i = 0, j = d_vertices.size() - 1; i < d_vertices.size(); j = i++)
      {
        Point const& a = d_vertices[i];
        Point const& b = d_vertices[j];
        LineSegment2<T> l(a, b);
        auto result = l.tryIntersect(line);
        if (result.hasValue())
        {
          // Ensure unique members of the list
          if (intersectionPoints.size() > 0 && intersectionPoints[0] == result.value())
            continue;
          if (intersectionPoints.size() > 1 && intersectionPoints[1] == result.value())
            continue;

          intersectionPoints.push_back(std::move(result.value()));

          // In a convex polygon, we should never see more than two intersections
          if (intersectionPoints.size() == 2)
            break;
        }
      }

      if (intersectionPoints.size() == 0)
      {
        // Line is completely outside and does not intersect with polygon
        return Maybe<LineSegment2<T>>::empty();
      }

      if (intersectionPoints.size() == 2)
      {
        // Line is completely outside and intersects with polygon
        return LineSegment2<T>(intersectionPoints[0], intersectionPoints[1]);
      }

      // One end of the line is outside, and we have a single intersection
      ASSERT(intersectionPoints.size() == 1);
      if (contains1)
        return LineSegment2<T>(line.p1(), intersectionPoints[0]);
      else
        return LineSegment2<T>(intersectionPoints[0], line.p2());
    }

  private:
    PointVector d_vertices;
  };

  typedef Polygon2<int> Polygon2i;
  typedef Polygon2<float> Polygon2f;
  typedef Polygon2<double> Polygon2d;
}
