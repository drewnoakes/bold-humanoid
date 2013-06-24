#include "fieldedgepass.hh"

#include "../../PixelLabel/pixellabel.hh"
#include "../../MovingAverage/movingaverage.hh"

using namespace bold;

FieldEdgePass::FieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight)
: d_fieldLabel(fieldLabel),
  d_maxYByX(pixelWidth),
  d_pixelWidth(pixelWidth),
  d_pixelHeight(pixelHeight)
{}

void FieldEdgePass::onImageStarting()
{
  assert(d_maxYByX.size() == d_pixelWidth);
  
  for (ushort x = 0; x < d_pixelWidth; x++)
    d_maxYByX[x] = d_pixelHeight - 1;
}

void FieldEdgePass::onPixel(uchar labelId, ushort x, ushort y)
{
  assert(x >= 0 && x < d_pixelWidth);
  
  if (labelId == d_fieldLabel->id())
  {
    assert(y >= d_maxYByX[x]);
    
    d_maxYByX[x] = y;
  }
}

uchar FieldEdgePass::getEdgeYValue(ushort x) const 
{
  assert(x < d_pixelWidth);
  assert(d_maxYByX[x] < d_pixelHeight);
  
  return d_maxYByX[x]; 

void FieldEdgePass::smooth(unsigned windowSize)
{
  MovingAverage<unsigned> avg(windowSize);

  for (int x = 0; x < d_pixelWidth; x++)
  {
    d_maxYByX[x] = avg.next(d_maxYByX[x]);
  }
}
