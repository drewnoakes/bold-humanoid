#include "Bounds2i.hh"

#include <vector>
#include <cassert>
#include <cmath>

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

#include "../util/Maybe.hh"
#include "../Colour/colour.hh"

#include "LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

using namespace bold;
using namespace cv;
using namespace Eigen;
using namespace std;

Bounds2i Bounds2i::merge(Bounds2i const& a, Bounds2i const& b)
{
  return Bounds2i(
    std::min(a.min().x(), b.min().x()),
    std::min(a.min().y(), b.min().y()),
    std::max(a.max().x(), b.max().x()),
    std::max(a.max().y(), b.max().y())
  );
}

int Bounds2i::minDimension() const
{
  return std::min(width(), height());
}

int Bounds2i::maxDimension() const
{
  return std::max(width(), height());
}

int Bounds2i::width() const
{
  return d_max.x() - d_min.x();
}

int Bounds2i::height() const
{
  return d_max.y() - d_min.y();
}

vector<Vector2i> Bounds2i::getCorners() const
{
  vector<Vector2i> corners = {
    d_min, Vector2i(d_min.x(), d_max.y()),
    d_max, Vector2i(d_max.x(), d_min.y())
  };

  return corners;
}

vector<LineSegment2i, aligned_allocator<LineSegment2i>> Bounds2i::getEdges() const
{
  auto corners = getCorners();
  vector<LineSegment2i, aligned_allocator<LineSegment2i>> edges;
  for (unsigned i = 0, lastIndex = 3; i < 4; lastIndex = i++)
  {
    if (corners[lastIndex] != corners[i])
      edges.push_back(LineSegment2i(corners[lastIndex], corners[i]));
  }

  return edges;
}
