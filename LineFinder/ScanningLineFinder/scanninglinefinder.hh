#pragma once

#include "../linefinder.hh"
#include "../../Config/config.hh"

namespace bold
{
  class ScanningLineFinder : public LineFinder
  {
  public:
    ScanningLineFinder();

    std::vector<LineSegment2i> findLineSegments(std::vector<Eigen::Vector2i>& lineDots) override;

  private:
    // Minimum line segment length required
    Setting<double>* d_minLength;
    // Minimum ratio of dots / length required
    Setting<double>* d_minCoverage;
    // Maximum root mean square error allowed
    Setting<double>* d_maxRMSError;
    // Maximum distance between new point and segment head
    Setting<double>* d_maxHeadDist;
    // Maximum distance between new point and line 
    Setting<double>* d_maxLineDist;
  };
}
