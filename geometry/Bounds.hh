#pragma once

#include <iostream>
#include <stdexcept>

#include <Eigen/Core>

namespace bold
{
  template <typename T,int dim>
  class Bounds
  {
  public:
    Bounds(Eigen::Matrix<T,dim,1> min, Eigen::Matrix<T,dim,1> max)
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

    Eigen::Matrix<T,dim,1> min() const { return d_min; }
    Eigen::Matrix<T,dim,1> max() const { return d_max; }

    bool contains(Eigen::Matrix<T,dim,1> const& v) const
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
    Eigen::Matrix<T,dim,1> d_min;
    Eigen::Matrix<T,dim,1> d_max;
  };

  typedef Bounds<double,2> Bounds2d;
  typedef Bounds<double,3> Bounds3d;
}
