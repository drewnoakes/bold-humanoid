#include "fieldedgepass.hh"

#include <Eigen/Core>

#include "../../Config/config.hh"
#include "../../geometry/halfhullbuilder.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

FieldEdgePass::FieldEdgePass(string id, ushort imageWidth, ushort imageHeight)
  : ImagePassHandler(id),
    d_imageWidth(imageWidth),
    d_imageHeight(imageHeight)
{
  Config::getSetting<int>("vision.field-edge-pass.min-vertical-run-length")->track([this](int value) { d_minVerticalRunLength = (ushort)value; });
  d_useConvexHull = Config::getSetting<bool>("vision.field-edge-pass.use-convex-hull");
}

void FieldEdgePass::applyConvexHull(vector<short>& points, unsigned fromIndex, unsigned toIndex)
{
  ASSERT(toIndex < points.size());

  vector<Matrix<float,2,1>> input;
  for (unsigned c = fromIndex; c <= toIndex; c++)
    input.emplace_back(c, points[c]);

  auto output = HalfHullBuilder<float>().findHalfHull(input, HalfHull::Top);

  // The convex hull output has fewer columns than the input.
  // Walk through both the columnar data and the hull output,
  // filling any missing column values with interpolations.

  unsigned outputIndex = 0;
  unsigned lastMatchedColumn = fromIndex;

  for (unsigned c = fromIndex; c <= toIndex; c++)
  {
    if (output[outputIndex].x() == c)
    {
      // This column's value is unchanged by the hull operation.
      // Its value is part of the hull

      for (unsigned fillC = lastMatchedColumn + 1; fillC < c; fillC++)
      {
        double ratio = 1.0 - (double)(c - fillC)/(c - lastMatchedColumn);
        points[fillC] = Math::lerp(ratio, points[lastMatchedColumn], points[c]);
      }

      lastMatchedColumn = c;
      outputIndex++;
    }
  }
}
