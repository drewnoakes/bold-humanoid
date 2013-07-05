#include "fieldedgepass.hh"

#include "../../PixelLabel/pixellabel.hh"
#include "../../MovingAverage/movingaverage.hh"

using namespace bold;

FieldEdgePass::FieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight)
: d_fieldLabel(fieldLabel),
  d_maxYByX(pixelWidth),
  d_runByX(pixelWidth),
  d_pixelWidth(pixelWidth),
  d_pixelHeight(pixelHeight),
  d_minVerticalRunLength(5)
{}

void FieldEdgePass::onImageStarting()
{
  assert(d_maxYByX.size() == d_pixelWidth);

  for (ushort x = 0; x < d_pixelWidth; x++)
    d_maxYByX[x] = d_pixelHeight - 1;

  memset(d_runByX.data(), 0, sizeof(ushort) * d_pixelWidth);
}

void FieldEdgePass::onPixel(uchar labelId, ushort x, ushort y)
{
//   assert(x >= 0 && x < d_pixelWidth);

  if (labelId == d_fieldLabel->id())
  {
//     assert(y >= d_maxYByX[x]);

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

ushort FieldEdgePass::getEdgeYValue(ushort x) const
{
  assert(x < d_pixelWidth);
  assert(d_maxYByX[x] < d_pixelHeight);

  return d_maxYByX[x];
}

void FieldEdgePass::smooth(unsigned windowSize)
{
  MovingAverage<unsigned> avg(windowSize);

  int offset = int(windowSize)/2;
  for (int x = 0, t = -offset; x < d_pixelWidth; x++, t++)
  {
    auto smoothedY = avg.next(d_maxYByX[x]);
    if (t >= 0 && smoothedY > d_maxYByX[t])
    {
      d_maxYByX[x] = smoothedY;
    }
  }
}
