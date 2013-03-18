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
using namespace std;


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

vector<LineSegment2i> Bounds2i::getEdges() const
{
  auto corners = getCorners();
  vector<LineSegment2i> edges;
  for (unsigned i = 0, lastIndex = 3; i < 4; lastIndex = i++)
  {
    if (corners[lastIndex] != corners[i])
      edges.push_back(LineSegment2i(corners[lastIndex], corners[i]));
  }

  return edges;
}
