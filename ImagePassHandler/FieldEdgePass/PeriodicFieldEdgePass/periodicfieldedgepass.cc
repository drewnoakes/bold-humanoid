#include "periodicfieldedgepass.hh"

#include "../../../Config/config.hh"
#include "../../../PixelLabel/pixellabel.hh"
#include "../../../stats/movingaverage.hh"

using namespace bold;
using namespace std;

PeriodicFieldEdgePass::PeriodicFieldEdgePass(shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight, ushort period)
: FieldEdgePass(fieldLabel, pixelWidth, pixelHeight),
  d_maxYByC((pixelWidth/period)+1),
  d_runByC((pixelWidth/period)+1),
  d_period(period)
{}

void PeriodicFieldEdgePass::onImageStarting()
{
  for (ushort c = 0; c < d_runByC.size(); c++)
    d_maxYByC[c] = d_pixelHeight - 1;

  memset(d_runByC.data(), 0, sizeof(ushort) * d_runByC.size());
}

void PeriodicFieldEdgePass::onPixel(uchar labelId, ushort x, ushort y)
{
//   assert(x >= 0 && x < d_pixelWidth);

  if (x % d_period != 0)
    return;

  ushort c = x / d_period;

  if (labelId == d_fieldLabel->id())
  {
//     assert(y >= d_maxYByX[c]);

    ushort run = d_runByC[c];
    run++;

    if (run >= d_minVerticalRunLength)
      d_maxYByC[c] = y;

    d_runByC[c] = run;
  }
  else
  {
    d_runByC[c] = 0;
  }
}

ushort PeriodicFieldEdgePass::getEdgeYValue(ushort x) const
{
  assert(x < d_pixelWidth);

  // Map from the x-postiion to the periodic samples.

  ushort rem = x % d_period;
  ushort c = x / d_period;

  if (rem == 0 || c == d_runByC.size() - 1)
  {
    // x is an exact multiple of the period, so return the value directly.
    assert(d_maxYByC[c] < d_pixelHeight);
    return d_maxYByC[c];
  }

  // Pixels at the far edge of the image may be beyond the last sampled column.
  if (c == d_maxYByC.size())
    return d_maxYByC[c - 1];

  // Interpolate between the two closest samples.
  return Math::lerp((double)rem/d_period, d_maxYByC[c], d_maxYByC[c + 1]);
}

void PeriodicFieldEdgePass::onImageComplete()
{
  if (d_useConvexHull->getValue())
    applyConvexHull(d_maxYByC);
}
