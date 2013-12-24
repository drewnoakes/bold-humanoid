#pragma once

#include <vector>
#include <type_traits>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../util/log.hh"

namespace bold
{
  typedef unsigned int uint;

  template<typename T>
  class Polygon2
  {
  public:
    typedef Eigen::Matrix<T, 2, 1> Point;

    Polygon2(std::vector<Point> vertices)
    : d_vertices(vertices)
    {
      if (vertices.size() < 3)
      {
        log::error("Polygon2::Polygon2") << "Cannot create a polygon with number of vertices: " << vertices.size();
        throw std::runtime_error("A polygon must have at least three vertices");
      }
    }

    bool contains(Point point)
    {
      bool isInside = false;
      for (int i = 0, j = d_vertices.size() - 1; i < d_vertices.size(); j = i++)
      {
        Point const& a = d_vertices[i];
        Point const& b = d_vertices[j];

        if (((a.y() > point.y()) != (b.y() > point.y()))
          && (point.x() < (b.x()-a.x()) * (point.y() - a.y()) / (b.y() - a.y()) + a.x()))
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

    typename std::vector<Point>::iterator begin() { return d_vertices.begin(); }
    typename std::vector<Point>::iterator end() { return d_vertices.end(); }
    typename std::vector<Point>::const_iterator begin() const { return d_vertices.begin(); }
    typename std::vector<Point>::const_iterator end() const { return d_vertices.end(); }

  private:
    std::vector<Point> d_vertices;
  };

  typedef Polygon2<int> Polygon2i;
  typedef Polygon2<float> Polygon2f;
  typedef Polygon2<double> Polygon2d;
}
