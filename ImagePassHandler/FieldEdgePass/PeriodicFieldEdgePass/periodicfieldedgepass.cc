#include "periodicfieldedgepass.hh"

#include "../../../Config/config.hh"
#include "../../../PixelLabel/pixellabel.hh"
#include "../../../SequentialTimer/sequentialtimer.hh"
#include "../../../stats/movingaverage.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

PeriodicFieldEdgePass::PeriodicFieldEdgePass(shared_ptr<PixelLabel> fieldLabel, shared_ptr<PixelLabel> lineLabel, ushort pixelWidth, ushort pixelHeight, ushort period)
: FieldEdgePass(pixelWidth, pixelHeight),
  d_fieldLabelId(fieldLabel->id()),
  d_lineLabelId(lineLabel->id()),
  d_maxYByC((pixelWidth/period)+1),
  d_maxYByCConvex((pixelWidth/period)+1),
  d_runByC((pixelWidth/period)+1),
  d_period(period)
{}

void PeriodicFieldEdgePass::onImageStarting(SequentialTimer& timer)
{
  for (ushort c = 0; c < d_runByC.size(); c++)
    d_maxYByC[c] = -1;

  memset(d_runByC.data(), 0, sizeof(ushort) * d_runByC.size());

  timer.timeEvent("Clear");
}

void PeriodicFieldEdgePass::onPixel(uchar labelId, ushort x, ushort y)
{
//   ASSERT(x >= 0 && x < d_pixelWidth);

  if (x % d_period != 0)
    return;

  ushort c = x / d_period;

  if (labelId == d_lineLabelId)
  {
    // Do nothing! Line may still be within field.
    // We don't increase the score however, and still reset at the first non-field/line pixel.
  }
  else if (labelId == d_fieldLabelId)
  {
//     ASSERT(y >= d_maxYByX[c]);

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
  ASSERT(x < d_pixelWidth);

  // Map from the x-position to the periodic samples.

  ushort rem = x % d_period;
  ushort c = x / d_period;

  auto const& maxYByC = d_useConvexHull->getValue() ? d_maxYByCConvex : d_maxYByC;

  if (rem == 0 || c == d_runByC.size() - 1)
  {
    // x is an exact multiple of the period, so return the value directly.
    ASSERT(maxYByC[c] < d_pixelHeight);
    return maxYByC[c];
  }

  // Pixels at the far edge of the image may be beyond the last sampled column.
  if (c == maxYByC.size())
    return maxYByC[c - 1];

  // Interpolate between the two closest samples.
  return Math::lerp((double)rem/d_period, maxYByC[c], maxYByC[c + 1]);
}

void PeriodicFieldEdgePass::onImageComplete(SequentialTimer& timer)
{
  // d_maxYByC is initialised with -1 in all positions.
  //
  // If we didn't observe a single column with enough green, then we will set
  // the entire line at of top the screen (when image is the right way up.)
  //
  // If we did see some good columns, take any that remain at -1 and set them
  // to the bottom of the screen.
  //
  // This stops the line jumping from the bottom to the top, which can happen
  // when the bot is near the side of the field and looking down its length.
  // The convex hull makes this issue much worse.

  bool allNegative = true;
  bool anyNegative = false;

  // If the edges are untouched (-1) we exclude them from the convex hull.
  // These variables track where the convex hull should start and end.
  int fromIndex = 0;
  int toIndex = d_maxYByC.size() - 1;

  for (ushort c = 0; c < d_maxYByC.size(); c++)
  {
    auto y = d_maxYByC[c];

    if (y == -1)
    {
      anyNegative = true;

      if (allNegative)
        fromIndex = c + 1;
    }
    else
    {
      allNegative = false;
      toIndex = c;
    }
  }

  if (allNegative)
  {
    // TODO control this via a setting, and disable by default -- if looking off the field, don't start chasing rubbish

    // set line at top of image
    for (ushort c = 0; c < d_runByC.size(); c++)
      d_maxYByC[c] = d_pixelHeight - 1;

    // skip convex hull as line is straight
    return;
  }
  else if (anyNegative)
  {
    // set untouched columns to the bottom of the image
    for (ushort c = 0; c < d_runByC.size(); c++)
    {
      if (d_maxYByC[c] == -1)
        d_maxYByC[c] = 0;
    }
  }

  // Create convex hull values
  std::copy(d_maxYByC.begin(), d_maxYByC.end(), d_maxYByCConvex.begin());
  if (fromIndex < toIndex)
    applyConvexHull(d_maxYByCConvex, fromIndex, toIndex);
  timer.timeEvent("Convex Hull");
}

vector<pair<Vector2i,Vector2i>> PeriodicFieldEdgePass::getOcclusionRays() const
{
  vector<pair<Vector2i,Vector2i>> deltas;

  ushort x = 0;
  for (ushort c = 0; c < d_maxYByC.size(); c++)
  {
    deltas.emplace_back(Vector2i(x, d_maxYByC[c]), Vector2i(x, d_maxYByCConvex[c]));
    x += d_period;
  }

  return deltas;
}
