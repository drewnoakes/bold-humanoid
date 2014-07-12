#include "completefieldedgepass.hh"

#include "../../../Config/config.hh"
#include "../../../PixelLabel/pixellabel.hh"
#include "../../../SequentialTimer/sequentialtimer.hh"
#include "../../../stats/movingaverage.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

CompleteFieldEdgePass::CompleteFieldEdgePass(shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight)
: FieldEdgePass(pixelWidth, pixelHeight),
  d_fieldLabelId(fieldLabel->id()),
  d_maxYByX(pixelWidth),
  d_runByX(pixelWidth)
{
  Config::getSetting<int>("vision.field-edge-pass.complete.smoothing-window-length")->track([this](int value) { d_smoothingWindowSize = (ushort)value; });
}

void CompleteFieldEdgePass::onImageStarting(SequentialTimer& timer)
{
  std::fill(d_maxYByX.begin(), d_maxYByX.end(), d_pixelHeight - 1);
  std::fill(d_runByX.begin(), d_runByX.end(), 0);
  timer.timeEvent("Clear");
}

void CompleteFieldEdgePass::onPixel(uchar labelId, ushort x, ushort y)
{
//   ASSERT(x >= 0 && x < d_pixelWidth);

  if (labelId == d_fieldLabelId)
  {
//     ASSERT(y >= d_maxYByX[x]);

    ushort run = d_runByX[x];
    run++;

    if (run >= d_minVerticalRunLength)
      d_maxYByX[x] = y;

    d_runByX[x] = run;
  }
  else
  {
    d_runByX[x] = 0;
  }
}

ushort CompleteFieldEdgePass::getEdgeYValue(ushort x) const
{
  ASSERT(x < d_pixelWidth);

  return d_useConvexHull->getValue() ? d_maxYByXConvex[x] : d_maxYByX[x];
}

void CompleteFieldEdgePass::onImageComplete(SequentialTimer& timer)
{
  if (d_smoothingWindowSize > 1)
  {
    MovingAverage<int> avg(d_smoothingWindowSize);

    int offset = int(d_smoothingWindowSize)/2;
    for (int x = 0, t = -offset; x < d_pixelWidth; x++, t++)
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
  applyConvexHull(d_maxYByXConvex, 0, d_pixelWidth - 1);
  timer.timeEvent("Convex Hull");
}

vector<OcclusionRay<ushort>> CompleteFieldEdgePass::getOcclusionRays() const
{
  vector<OcclusionRay<ushort>> rays;

  for (ushort x = 0; x < d_pixelWidth; x++)
    rays.emplace_back(
      Matrix<ushort,2,1>(x, d_maxYByX[x]),
      Matrix<ushort,2,1>(x, d_maxYByXConvex[x]));

  return rays;
}
