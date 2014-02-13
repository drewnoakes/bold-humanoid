#include "fieldedgepass.hh"

#include <Eigen/Core>

#include "../../Config/config.hh"
#include "../../geometry/halfhullbuilder.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

FieldEdgePass::FieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight)
: d_fieldLabel(fieldLabel),
  d_pixelWidth(pixelWidth),
  d_pixelHeight(pixelHeight)
{
  Config::getSetting<int>("vision.field-edge-pass.min-vertical-run-length")->track([this](int value) { d_minVerticalRunLength = value; });
  d_useConvexHull = Config::getSetting<bool>("vision.field-edge-pass.use-convex-hull");
}

void FieldEdgePass::applyConvexHull(vector<ushort>& points)
{
  vector<Matrix<float,2,1>> input;
  for (unsigned c = 0; c < points.size(); c++)
    input.push_back(Matrix<float,2,1>(c, points[c]));

  auto output = HalfHullBuilder<float>().findHalfHull(input, HalfHull::Top);

  // The convex hull output has fewer columns than the input.
  // Walk through both the columnar data and the hull output,
  // filling any missing column values with interpolations.

  int outputIndex = 0;
  int lastMatchedColumn = 0;

  for (unsigned c = 0; c < points.size(); c++)
  {
    if (output[outputIndex].x() == c)
    {
      // This column's value is unchanged by the hull operation.
      // Its value is part of the hull

      for (int fillC = lastMatchedColumn + 1; fillC < c; fillC++)
      {
        double ratio = 1.0 - (double)(c - fillC)/(c - lastMatchedColumn);
        points[fillC] = Math::lerp(ratio, points[lastMatchedColumn], points[c]);
      }

      lastMatchedColumn = c;
      outputIndex++;
    }
  }
}
