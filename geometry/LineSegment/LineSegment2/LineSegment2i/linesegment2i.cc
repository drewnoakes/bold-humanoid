#include "linesegment2i.hh"

#include <cmath>
#include <limits>
#include <vector>
#include <set>

#include "../../../../util/assert.hh"
#include "../../../../util/Maybe.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

double LineSegment2i::gradient() const
{
  auto d = delta();
  if (d.x() == 0)
    return numeric_limits<double>::infinity();
  return d.y() / (double)d.x();
}

double LineSegment2i::yIntersection() const
{
  if (p1().x() == p2().x())
    return numeric_limits<double>::quiet_NaN();
  return double(p1().y()) - gradient() * double(p1().x());
}

double LineSegment2i::angle() const
{
  auto delta = this->delta();
  return atan2(delta.y(), delta.x());
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

  auto edges = bounds.getEdges();

  // we use a set, as duplicates can be created if lines pass through corners
  set<Vector2i, Vector2iCompare> ends;

  // Add ends if they're in bounds
  if (bounds.contains(p1()))
    ends.insert(p1());
  if (bounds.contains(p2()))
    ends.insert(p2());

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

  ASSERT(ends.size() <= 2);
  ASSERT(ends.size() != 1);

  vector<Vector2i> endsVec(ends.begin(), ends.end());

  return ends.size() == 2
    ? Maybe<LineSegment2i>(LineSegment2i(endsVec[0], endsVec[1]))
    : Maybe<LineSegment2i>::empty();
}
