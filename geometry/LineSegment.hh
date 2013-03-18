#ifndef BOLD_LINE_SEGMENT_3D_HH
#define BOLD_LINE_SEGMENT_3D_HH

#include <Eigen/Core>
#include <stdexcept>

namespace bold
{
  template<typename T,int dim>
  struct LineSegment
  {
  public:
    LineSegment(Eigen::Matrix<T,dim,1> const& p1,
                Eigen::Matrix<T,dim,1> const& p2)
    : d_p1(p1),
      d_p2(p2)
    {
      if ((p2 - p1).cwiseAbs().maxCoeff() == 0)
        throw std::runtime_error("Points must have different values.");
    }

    Eigen::Matrix<T,dim,1> p1() const { return d_p1; }
    Eigen::Matrix<T,dim,1> p2() const { return d_p2; }

    /** Returns the vector formed by <code>p2() - p1()</code> */
    Eigen::Matrix<T,dim,1> delta() const { return d_p2 - d_p1; }

    template<int newDim>
    LineSegment<T,newDim> to() const
    {
      Eigen::Matrix<T,newDim,1> newp1 = Eigen::Matrix<T,newDim,1>::Zero();;
      Eigen::Matrix<T,newDim,1> newp2 = Eigen::Matrix<T,newDim,1>::Zero();;

      newp1.template head< MIN<dim,newDim>::val >() = d_p1.template head< MIN<dim,newDim>::val >();
      newp2.template head< MIN<dim,newDim>::val >() = d_p2.template head< MIN<dim,newDim>::val >();

      return LineSegment<T,newDim>(newp1, newp2);
    }

    template<typename newT>
    LineSegment<newT,dim> cast()
    {
      return LineSegment<newT,dim>(d_p1.cast<newT>(),
                                   d_p2.cast<newT>());
    }

    bool operator==(LineSegment<T,dim> const& other) const
    {
      const double epsilon = 0.0000004;

      return ((d_p1 - other.d_p1).cwiseAbs() + (d_p2 - other.d_p2).cwiseAbs()).sum() < epsilon;
    }

    friend std::ostream& operator<<(std::ostream& stream, LineSegment<T,dim> const& lineSegment)
    {
      return stream << "LineSegment (P1=" << lineSegment.d_p1.transpose() << " P2=" << lineSegment.d_p2.transpose() << ")";
    }

  protected:
    Eigen::Matrix<T,dim,1> d_p1;
    Eigen::Matrix<T,dim,1> d_p2;

  private:
    template <bool COND, int A, int B>
    struct IF
    {
      enum { val = A };
    };

    template <int A, int B>
    struct IF<false, A, B>
    {
      enum { val = B };
    };

    template <int A, int B>
    struct MIN : IF<A < B, A, B>
    {};
  };

  typedef LineSegment<double,2> LineSegment2d;
  typedef LineSegment<double,3> LineSegment3d;
  typedef LineSegment<int,3> LineSegment3i;
}

#endif