#include "Bounds2i.hh"

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

#include "../util/Maybe.hh"
#include "../Colour/colour.hh"

#include "LineSegment2i.hh"

using namespace bold;
using namespace cv;
using namespace Eigen;

Bounds2i::Bounds2i(int minX, int minY, int maxX, int maxY)
: d_min(Eigen::Vector2i(minX, minY)),
  d_max(Eigen::Vector2i(maxX, maxY))
{
  assert(d_min.x() <= d_max.x() && d_min.y() <= d_max.y());
}

Bounds2i::Bounds2i(Eigen::Vector2i min, Eigen::Vector2i max)
: d_min(min),
  d_max(max)
{
  assert(d_min.x() <= d_max.x() && d_min.y() <= d_max.y());
}

bool Bounds2i::contains(Eigen::Vector2i const& v) const
{
  return v.x() >= d_min.x()
      && v.x() <= d_max.x()
      && v.y() >= d_min.y()
      && v.y() <= d_max.y();
}

int Bounds2i::width() const
{
  return d_max.x() - d_min.x();
}

int Bounds2i::height() const
{
  return d_max.y() - d_min.y();
}

bool Bounds2i::isEmpty() const
{
  return width() != 0 && height() != 0;
}

std::vector<Eigen::Vector2i> Bounds2i::getCorners() const
{
  std::vector<Eigen::Vector2i> corners = {
    d_min, Eigen::Vector2i(d_min.x(), d_max.y()),
    d_max, Eigen::Vector2i(d_max.x(), d_min.y())
  };

  return corners;
}

std::vector<LineSegment2i> Bounds2i::getEdges() const
{
  auto corners = getCorners();
  std::vector<LineSegment2i> edges;
  for (unsigned i = 0, lastIndex = 3; i < 4; i++)
  {
    if (corners[lastIndex] != corners[i])
      edges.push_back(LineSegment2i(corners[lastIndex], corners[i]));
    lastIndex = i;
  }

  return edges;
}
