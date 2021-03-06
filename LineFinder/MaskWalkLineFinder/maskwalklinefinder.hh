#pragma once

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include "../../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"
#include "../linefinder.hh"

namespace bold
{
  template<typename> class Setting;

  class MaskWalkLineFinder : public LineFinder
  {
  public:
    MaskWalkLineFinder();

    void walkLine(Eigen::Vector2i const& start, float theta, bool forward, std::function<bool(int/*x*/,int/*y*/)> const& callback, uchar width = 1);

    std::vector<LineSegment2i> findLineSegments(std::vector<Eigen::Vector2i>& lineDots) override;

  private:
    void rebuild();

    // configuration options
    Setting<double>* d_drThreshold;
    Setting<double>* d_dtThresholdDegs;
    Setting<int>* d_voteThreshold;
    Setting<int>* d_minLineLength;
    Setting<int>* d_maxLineGap;
    Setting<int>* d_maxLineSegmentCount;

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
  };
}
