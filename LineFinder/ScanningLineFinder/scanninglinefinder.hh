#pragma once

#include "../linefinder.hh"

namespace bold
{
  class ScanningLineFinder : public LineFinder
  {
  public:
    ScanningLineFinder() {}

    std::vector<LineSegment2i> findLineSegments(std::vector<Eigen::Vector2i>& lineDots) override;
  };
}
