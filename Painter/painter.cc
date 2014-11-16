#include "painter.hh"

#include <opencv2/core/core.hpp>

#include "../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

using namespace bold;
using namespace cv;

void Painter::draw(LineSegment2i const& segment, Mat& img, Scalar const& colour, unsigned thickness)
{
  line(img,
       Point(segment.p1().x(), segment.p1().y()),
       Point(segment.p2().x(), segment.p2().y()),
       colour, thickness);
}

