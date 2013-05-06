#include "LineSegment2i.hh"

#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <cmath>
#include <stdexcept>

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

#include "../util/Maybe.hh"
#include "../Colour/colour.hh"

#include "Line.hh"

using namespace bold;
using namespace cv;
using namespace Eigen;
using namespace std;

double LineSegment2i::fake2dCross(Vector2d const& a, Vector2d const& b)
{
  return a.x()*b.y() - a.y()*b.x();
}

double LineSegment2i::gradient() const
{
  auto d = delta();
  if (d.x() == 0)
    return FP_INFINITE;
  return d.y() / (double)d.x();
}

double LineSegment2i::yIntersection() const
{
  if (d_p1.x() == d_p2.x())
    return FP_NAN;
  return (double)d_p1.y() - gradient() * (double)d_p1.x();
}

double LineSegment2i::angle() const
{
  auto delta = this->delta();
  return atan2(delta.y(), delta.x());
}

void LineSegment2i::draw(Mat& image, Colour::bgr const& bgr, int thickness) const
{
  line(image, Point(d_p1.x(), d_p1.y()), Point(d_p2.x(), d_p2.y()), bgr.toScalar(), thickness);
}

Line LineSegment2i::toLine() const
{
  double theta = atan2(d_p2.y() - d_p1.y(), d_p1.x() - d_p2.x());

  double radius = d_p1.x()*sin(theta) + d_p1.y()*cos(theta);

  while (theta < 0)
  {
    theta += M_PI;
    radius = -radius;
  }

  while (theta > M_PI)
  {
    theta -= M_PI;
    radius = -radius;
  }

  return Line(radius, theta);
}

Maybe<LineSegment2i> LineSegment2i::cropTo(Bounds2i const& bounds) const
{
  struct Vector2iCompare
  {
    bool operator() (Vector2i const& lhs, Vector2i const& rhs) const
    {
      if (lhs.x() == rhs.x())
        return lhs.y() < rhs.y();

      return lhs.x() < rhs.x();
    }
  };

  vector<LineSegment2i> edges = bounds.getEdges();

  // we use a set, as duplicates can be created if lines pass through corners
  set<Vector2i, Vector2iCompare> ends;

  // Add ends if they're in bounds
  if (bounds.contains(d_p1))
    ends.insert(d_p1);
  if (bounds.contains(d_p2))
    ends.insert(d_p2);

  // If we have two ends, we don't need to test for intersections with the edges
  if (ends.size() != 2)
  {
    for (LineSegment2i const& edge : edges)
    {
      Maybe<Vector2i> intersect = tryIntersect(edge);
      if (intersect.hasValue())
      {
        ends.insert(*intersect);
      }
    }
  }

  assert(ends.size() <= 2);
  assert(ends.size() != 1);

  vector<Vector2i> endsVec(ends.begin(), ends.end());

  return ends.size() == 2
    ? Maybe<LineSegment2i>(LineSegment2i(endsVec[0], endsVec[1]))
    : Maybe<LineSegment2i>::empty();
}

Maybe<Vector2i> LineSegment2i::tryIntersect(LineSegment2i const& other) const
{
  // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

  Vector2d pos1 = d_p1.cast<double>(); // p
  Vector2d dir1 = delta().cast<double>(); // r
  Vector2d pos2 = other.d_p1.cast<double>(); // q
  Vector2d dir2 = other.delta().cast<double>(); // s

  double denom = fake2dCross(dir1, dir2);
  double numer = fake2dCross(pos2 - pos1, dir2);

  if (/*numer == 0 ||*/ denom == 0)
  {
    // lines are collinear or parallel
    return Maybe<Vector2i>::empty();
  }

  double t = numer / denom;

  if (t < 0 || t > 1)
  {
    // line segments do not intersect within their ranges
    return Maybe<Vector2i>::empty();
  }

  Vector2d intersectionPoint = pos1 + dir1 * t;

  Vector2d rounded = intersectionPoint.unaryExpr([](double v) { return round(v); });

  return Maybe<Vector2i>(rounded.cast<int>());
}
