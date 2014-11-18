#pragma once

#include "../../util/assert.hh"
#include "../../util/meta.hh"

#include <Eigen/Core>
#include <stdexcept>
#include <sstream>

namespace bold
{
  template<typename T,int dim>
  struct LineSegment : Eigen::Matrix<T,dim,2>
  {
  public:
    typedef Eigen::Matrix<T,dim,2> Base;
    typedef Eigen::Matrix<T,dim,1> PointType;

    template<typename OtherDerived>
    LineSegment(Eigen::MatrixBase<OtherDerived> const& other)
      :  Base(other)
    {}

    LineSegment(PointType const& p1,
                PointType const& p2)
    {
      static_assert(std::is_arithmetic<T>::value, "Must be an arithmetic type");

      this->col(0) = p1;
      this->col(1) = p2;

      ASSERT((p2 - p1).cwiseAbs().maxCoeff() != 0);
    }

    template<typename OtherDerived>
    LineSegment& operator=(Eigen::MatrixBase<OtherDerived> const& other)
    {
      this->Base::operator=(other);
      return *this;
    }

    PointType p1() const { return this->col(0); }
    PointType p2() const { return this->col(1); }

    /** Returns the vector formed by <code>p2() - p1()</code> */
    PointType delta() const { return p2() - p1(); }

    /** Returns the vector formed by <code>(p2() + p1()) / 2.0</code> */
    PointType mid() const { return (p2() + p1()) / 2.0; }

    T length() const { return delta().norm(); }

    double normalisedDot(LineSegment<T,dim> other) const
    {
      return delta().normalized().dot( other.delta().normalized() );
    }

    /** Calculate the smallest angle between the two line segments.
     *
     * The output will be between 0 and PI/2, inclusive.
     */
    double smallestAngleBetween(LineSegment<T,dim> other)
    {
      auto a = fabs(acos(normalisedDot(other)));
      if (a >= M_PI/2)
        a = M_PI - a;
      return a;
    }

    template<int newDim>
    LineSegment<T,newDim> to() const
    {
      auto newLineSegment = LineSegment<T,newDim>{LineSegment<T,newDim>::Zero()};
      newLineSegment.col(0).template head< meta::min<dim,newDim>::value >() =
        this->col(0).template head< meta::min<dim,newDim>::value >();
      newLineSegment.col(1).template head< meta::min<dim,newDim>::value >() =
        this->col(1).template head< meta::min<dim,newDim>::value >();
      return newLineSegment;
    }

    bool operator==(LineSegment<T,dim> const& other) const
    {
      const double epsilon = 0.0000004;
      return (this->array() - other.array()).abs().sum() < epsilon;
    }

    LineSegment<T,dim> operator+(PointType const& delta) const
    {
      return LineSegment<T,dim>{this->colwise() + delta};
    }

    LineSegment<T,dim> operator-(PointType const& delta) const
    {
      return LineSegment<T,dim>{this->colwise() - delta};
    }

    friend std::ostream& operator<<(std::ostream& stream, LineSegment<T,dim> const& lineSegment)
    {
      return stream << "LineSegment (P1=" << lineSegment.p1().transpose() << " P2=" << lineSegment.p2().transpose() << ")";
    }

  };

  typedef LineSegment<double,3> LineSegment3d;
  typedef LineSegment<int,3> LineSegment3i;
}
