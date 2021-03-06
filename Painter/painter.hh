#pragma once

#include <opencv2/core/core.hpp>

#include "../Colour/colour.hh"

namespace bold
{
  struct LineSegment2i;

  class Painter
  {
  public:
    static void draw(LineSegment2i const& segment, cv::Mat& img, cv::Scalar const& colour, unsigned thickness);
  };
}
