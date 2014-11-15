#include "completefieldedgepass.hh"

#include "../../../Config/config.hh"
#include "../../../ImageLabelData/imagelabeldata.hh"
#include "../../../PixelLabel/pixellabel.hh"
#include "../../../SequentialTimer/sequentialtimer.hh"
#include "../../../stats/movingaverage.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

CompleteFieldEdgePass::CompleteFieldEdgePass(shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight)
: FieldEdgePass("CompleteFieldEdgePass", pixelWidth, pixelHeight),
  d_fieldLabelId((uint8_t)fieldLabel->getID()),
  d_maxYByX(pixelWidth),
  d_runByX(pixelWidth)
{
  Config::getSetting<int>("vision.field-edge-pass.complete.smoothing-window-length")->track([this](int value) { d_smoothingWindowSize = (ushort)value; });
}

void CompleteFieldEdgePass::process(ImageLabelData const& labelData, SequentialTimer& timer)
{
  std::fill(d_maxYByX.begin(), d_maxYByX.end(), d_imageHeight - 1);
  std::fill(d_runByX.begin(), d_runByX.end(), 0);
  timer.timeEvent("Clear");

  for (auto const& row : labelData)
  {
    ushort x = 0;
    for (auto const& label : row)
    {
//   ASSERT(x >= 0 && x < d_imageWidth);

      if (label == d_fieldLabelId)
      {
//     ASSERT(y >= d_maxYByX[x]);

        ushort run = d_runByX[x];
        run++;

        if (run >= d_minVerticalRunLength)
          d_maxYByX[x] = row.imageY;

        d_runByX[x] = run;
      }
      else
      {
        d_runByX[x] = 0;
      }

      x += row.granularity.x();
    }
  }
  timer.timeEvent("Process Rows");

  if (d_smoothingWindowSize > 1)
  {
    MovingAverage<int> avg(d_smoothingWindowSize);

    int offset = int(d_smoothingWindowSize)/2;
    for (int x = 0, t = -offset; x < d_imageWidth; x++, t++)
    {
      auto smoothedY = avg.next(d_maxYByX[x]);
      if (t >= 0 && smoothedY > d_maxYByX[t])
      {
        d_maxYByX[x] = smoothedY;
      }
    }

    timer.timeEvent("Smooth");
  }

  // Create convex hull values
  std::copy(d_maxYByX.begin(), d_maxYByX.end(), d_maxYByXConvex.begin());
  applyConvexHull(d_maxYByXConvex, 0, d_imageWidth - 1);
  timer.timeEvent("Convex Hull");
}

ushort CompleteFieldEdgePass::getEdgeYValue(ushort x) const
{
  ASSERT(x < d_imageWidth);

  return d_useConvexHull->getValue() ? d_maxYByXConvex[x] : d_maxYByX[x];
}

vector<OcclusionRay<ushort>> CompleteFieldEdgePass::getOcclusionRays() const
{
  vector<OcclusionRay<ushort>> rays;

  for (ushort x = 0; x < d_imageWidth; x++)
    rays.emplace_back(
      Matrix<ushort,2,1>(x, d_maxYByX[x]),
      Matrix<ushort,2,1>(x, d_maxYByXConvex[x]));

  return rays;
}
