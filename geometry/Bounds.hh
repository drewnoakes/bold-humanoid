#pragma once

#include <stdexcept>
#include <vector>

#include <Eigen/Core>

#include "LineSegment/linesegment.hh"

namespace bold
{
  template <typename T,int dim>
  class Bounds
  {
  public:
    typedef Eigen::Matrix<T,dim,1> Point;

    Bounds(Point min, Point max)
    : d_min(min),
      d_max(max)
    {
      if ((max - min).minCoeff() < 0)
        throw std::runtime_error("Max must be greater than min.");
    }

    bool operator==(Bounds<T,dim> const& other) const
    {
      return d_min == other.d_min && d_max == other.d_max;
    }

    friend std::ostream& operator<<(std::ostream& stream, Bounds<T,dim> const& line)
    {
      return stream << "Bounds (min=" << line.d_min.transpose() << " max=" << line.d_max.transpose() << ")";
    }

    Point min() const { return d_min; }
    Point max() const { return d_max; }

    Point mid() const
    {
      Eigen::Matrix<T,dim,1> mid;
      for (int i = 0; i < dim; i++)
        mid[i] = (d_min[i] + d_max[i]) / 2;
      return mid;
    }

    bool contains(Point const& v) const
    {
      return (v - d_min).minCoeff() >= 0 && (d_max - v).minCoeff() >= 0;
    }

    bool overlaps(Bounds<T,dim> const& other) const
    {
      for (int i = 0; i < dim; i++)
      {
        if (d_max[i] <= other.d_min[i] ||
            d_min[i] >= other.d_max[i])
          return false;
      }

      return true;
    }

    /** True if the any dimension of this bounding box are zero. */
    bool isEmpty() const
    {
      return (d_max - d_min).cwiseAbs().minCoeff() == 0;
    }

  protected:
    Point d_min;
    Point d_max;
  };

  template<typename T>
  class Bounds2 : public Bounds<T,2>
  {
  public:
    typedef Eigen::Matrix<T,2,1> Point;
    typedef LineSegment<T,2> LineSegmentType;

    static Bounds2<T> merge(Bounds2<T> const& a, Bounds2<T> const& b)
    {
      return Bounds2<T>(
        std::min(a.min().x(), b.min().x()),
        std::min(a.min().y(), b.min().y()),
        std::max(a.max().x(), b.max().x()),
        std::max(a.max().y(), b.max().y()));
    }

    Bounds2(T minX, T minY, T maxX, T maxY)
      : Bounds<T,2>::Bounds(Point(minX, minY), Point(maxX, maxY))
    {}

    Bounds2(Point minPoint, Point maxPoint)
      : Bounds<T,2>::Bounds(minPoint, maxPoint)
    {}

    T minDimension() const
    {
      return std::min(width(), height());
    }

    T maxDimension() const
    {
      return std::max(width(), height());
    }

    T width() const
    {
      return this->d_max.x() - this->d_min.x();
    }

    T height() const
    {
      return this->d_max.y() - this->d_min.y();
    }

    /** Returns corners in clockwise order, starting at 'min'. */
    std::vector<Point> getCorners() const
    {
      std::vector<Point> corners = {
        this->d_min, Point(this->d_min.x(), this->d_max.y()),
        this->d_max, Point(this->d_max.x(), this->d_min.y())
      };

      return corners;
    }

    std::vector<LineSegmentType, Eigen::aligned_allocator<LineSegmentType>> getEdges() const
    {
      auto corners = getCorners();
      std::vector<LineSegmentType, Eigen::aligned_allocator<LineSegmentType>> edges;
      for (unsigned i = 0, lastIndex = 3; i < 4; lastIndex = i++)
      {
        if (corners[lastIndex] != corners[i])
          edges.emplace_back(corners[lastIndex], corners[i]);
      }

      return edges;
    }
  };

  typedef Bounds2<int> Bounds2i;
  typedef Bounds2<double> Bounds2d;
  typedef Bounds<double,3> Bounds3d;
}
