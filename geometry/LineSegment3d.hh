#ifndef BOLD_LINE_SEGMENT_3D_HH
#define BOLD_LINE_SEGMENT_3D_HH

#include <Eigen/Core>

namespace bold
{
  struct LineSegment3d
  {
  public:
    LineSegment3d(Eigen::Vector3d p1,
                  Eigen::Vector3d p2)
    : d_p1(p1),
      d_p2(p2)
    {
      if (p1.x() == p2.x() && p1.y() == p2.y() && p1.z() == p2.z())
        throw std::string("Points must have different values.");
    }

    LineSegment3d(double x1, double y1, double z1,
                  double x2, double y2, double z2)
    : d_p1(Vector3d(x1, y1, z1)),
      d_p2(Vector3d(x2, y2, z2))
    {
      if (x1 == x2 && y1 == y2 && z1 == z2)
        throw std::string("Points must have different values.");
    }

    Eigen::Vector3d p1() const { return d_p1; }
    Eigen::Vector3d p2() const { return d_p2; }

    /** Returns the vector formed by p2() - p1() */
    Eigen::Vector3d delta() const { return d_p2 - d_p1; }

    bool operator==(LineSegment3d const& other) const
    {
      const double epsilon = 0.0000001;
      return fabs(d_p1.x() - other.d_p1.x()) < epsilon
          && fabs(d_p1.y() - other.d_p1.y()) < epsilon
          && fabs(d_p1.z() - other.d_p1.z()) < epsilon
          && fabs(d_p2.x() - other.d_p2.x()) < epsilon
          && fabs(d_p2.y() - other.d_p2.y()) < epsilon
          && fabs(d_p2.z() - other.d_p2.z()) < epsilon;
    }

    friend std::ostream& operator<<(std::ostream& stream, LineSegment3d const& lineSegment)
    {
      return stream << "LineSegment3d (P1=" << lineSegment.d_p1.transpose() << " P2=" << lineSegment.d_p2.transpose() << ")";
    }

  private:
    Eigen::Vector3d d_p1;
    Eigen::Vector3d d_p2;
  };
}

#endif