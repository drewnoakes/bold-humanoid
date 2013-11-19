#include "fieldedgepass.hh"

#include "../../Config/config.hh"
#include "../../MovingAverage/movingaverage.hh"
#include "../../PixelLabel/pixellabel.hh"

using namespace bold;
using namespace std;

FieldEdgePass::FieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight)
: d_fieldLabel(fieldLabel),
  d_maxYByX(pixelWidth),
  d_runByX(pixelWidth),
  d_pixelWidth(pixelWidth),
  d_pixelHeight(pixelHeight),
  d_smoothingWindowSize(Config::getValue<int>("vision.field-edge-pass.smoothing-window-length")),
  d_minVerticalRunLength(Config::getValue<int>("vision.field-edge-pass.min-vertical-run-length"))
{
  Config::getSetting<int>("vision.field-edge-pass.smoothing-window-length")->changed.connect([this](int value) { d_smoothingWindowSize = value; });
  Config::getSetting<int>("vision.field-edge-pass.min-vertical-run-length")->changed.connect([this](int value) { d_minVerticalRunLength = value; });
}

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

void FieldEdgePass::onImageComplete()
{
  // Bail out early if no smoothing is requested
  if (d_smoothingWindowSize == 1)
    return;

  MovingAverage<unsigned> avg(d_smoothingWindowSize);

  int offset = int(d_smoothingWindowSize)/2;
  for (int x = 0, t = -offset; x < d_pixelWidth; x++, t++)
  {
    auto smoothedY = avg.next(d_maxYByX[x]);
    if (t >= 0 && smoothedY > d_maxYByX[t])
    {
      d_maxYByX[x] = smoothedY;
    }
  }
}
