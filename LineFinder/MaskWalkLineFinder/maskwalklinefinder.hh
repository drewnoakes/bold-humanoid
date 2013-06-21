#pragma once

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include "../../Control/control.hh"
#include "../../geometry/LineSegment2i.hh"
#include "../linefinder.hh"

class minIni;

namespace bold
{
  class MaskWalkLineFinder : public LineFinder
  {
  public:
    MaskWalkLineFinder(int const imageWidth, const int imageHeight);

    void initialise(minIni const& ini);

    void walkLine(Eigen::Vector2i const& start, float theta, bool forward, std::function<bool(int/*x*/,int/*y*/)> const& pred);

    std::vector<LineSegment2i> findLineSegments(std::vector<Eigen::Vector2i>& lineDots) override;

    std::vector<std::shared_ptr<Control const>> getControls() const override { return d_controls; }

  private:
    void rebuild();

    // configuration options
    float d_drThreshold;
    float d_dtThreshold;
    int d_voteThreshold;
    int d_minLineLength;
    int d_maxLineGap;
    int d_maxLineSegmentCount;

    // constant
    const int d_imageWidth;
    const int d_imageHeight;

    // cached
    cv::Mat d_mask;
    cv::Mat d_accumulator;

    // calculated
    int d_tSteps;
    int d_rSteps;
    std::vector<float> d_trigTable;

    // controls
    std::vector<std::shared_ptr<Control const>> d_controls;
  };
}
