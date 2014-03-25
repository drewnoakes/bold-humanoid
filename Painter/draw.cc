#include "painter.ih"

void Painter::draw(LineSegment2i const& segment, cv::Mat& img, Colour::bgr const& colour, unsigned thickness)
{
  cv::line(img,
           cv::Point(segment.p1().x(), segment.p1().y()),
           cv::Point(segment.p2().x(), segment.p2().y()),
           colour.toScalar(), thickness);
}

