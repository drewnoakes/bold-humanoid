#pragma once

#include "../linefinder.hh"
#include "../../Config/config.hh"

namespace bold
{
  class CameraModel;

  class ScanningLineFinder : public LineFinder
  {
  public:
    ScanningLineFinder(std::shared_ptr<CameraModel> cameraModel);

    std::vector<LineSegment2i> findLineSegments(std::vector<Eigen::Vector2i>& lineDots) override;

  private:
    std::shared_ptr<CameraModel> d_cameraModel;

    // Minimum line segment length required
    Setting<double>* d_minLength;
    // Minimum ratio of dots / length required
    Setting<double>* d_minCoverage;
    // Maximum root mean square error allowed
    Setting<double>* d_maxRMSFactor;
    // Maximum distance between new point and segment head
    Setting<double>* d_maxHeadDist;
    // Maximum distance between new point and line 
    Setting<double>* d_maxLineDist;
  };
}
