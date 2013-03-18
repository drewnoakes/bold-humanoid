#ifndef LINEFINDER_HH
#define LINEFINDER_HH

#include <vector>
#include <Eigen/Core>

#include "../Control/control.hh"
#include "../geometry/LineSegment2i.hh"

namespace bold
{
  class LineFinder
  {
  public:
    virtual std::vector<LineSegment2i> findLineSegments(std::vector<Eigen::Vector2i>& lineDots) {};

    virtual std::vector<bold::Control> getControls() const {};
  };
}

#endif // LINEFINDER_HH
