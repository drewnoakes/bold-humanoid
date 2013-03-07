#ifndef BOLD_LINE_SEGMENT_2D_HH
#define BOLD_LINE_SEGMENT_2D_HH

#include <Eigen/Core>

namespace bold
{
  // TODO can this be merged with LineSegment2i using templates & typedef?

  struct LineSegment2d
  {
  public:
    LineSegment2d(Eigen::Vector2d p1,
                  Eigen::Vector2d p2)
    : d_p1(p1),
      d_p2(p2)
    {
      if (p1.x() == p2.x() && p1.y() == p2.y())
        throw std::string("Points must have different values.");
    }

    LineSegment2d(double x1, double y1,
                  double x2, double y2)
    : d_p1(Eigen::Vector2d(x1, y1)),
      d_p2(Eigen::Vector2d(x2, y2))
    {
      if (x1 == x2 && y1 == y2)
        throw std::string("Points must have different values.");
    }

    Eigen::Vector2d p1() const { return d_p1; }
    Eigen::Vector2d p2() const { return d_p2; }

    /** Returns the vector formed by p2() - p1() */
    Eigen::Vector2d delta() const { return d_p2 - d_p1; }

    bool operator==(LineSegment2d const& other) const
    {
      const double epsilon = 0.0000001;
      return fabs(d_p1.x() - other.d_p1.x()) < epsilon
          && fabs(d_p1.y() - other.d_p1.y()) < epsilon
          && fabs(d_p2.x() - other.d_p2.x()) < epsilon
          && fabs(d_p2.y() - other.d_p2.y()) < epsilon;
    }

    friend std::ostream& operator<<(std::ostream& stream, LineSegment2d const& lineSegment)
    {
      return stream << "LineSegment2d (P1=" << lineSegment.d_p1.transpose() << " P2=" << lineSegment.d_p2.transpose() << ")";
    }

  private:
    Eigen::Vector2d d_p1;
    Eigen::Vector2d d_p2;
  };
}

#endif