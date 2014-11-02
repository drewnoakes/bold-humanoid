#include "painter.ih"

void Painter::draw(LineSegment2i const& segment, cv::Mat& img, cv::Scalar const& colour, unsigned thickness)
{
  cv::line(img,
           cv::Point(segment.p1().x(), segment.p1().y()),
           cv::Point(segment.p2().x(), segment.p2().y()),
           colour, thickness);
}

