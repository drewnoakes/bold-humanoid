#pragma once

#include <memory>

#include "../imagepasshandler.hh"
#include "../../PixelLabel/pixellabel.hh"

namespace bold
{
  class FieldEdgePass : public ImagePassHandler<uchar>
  {
  public:
    FieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight)
    : d_fieldLabel(fieldLabel),
      d_pixelWidth(pixelWidth),
      d_pixelHeight(pixelHeight),
      d_maxYByX(pixelWidth)
    {}

    void onImageStarting() override
    {
      assert(d_maxYByX.size() == d_pixelWidth);
      
      for (ushort x = 0; x < d_pixelWidth; x++)
        d_maxYByX[x] = d_pixelHeight;
    }

    void onPixel(uchar labelId, ushort x, ushort y) override
    {
      assert(x >=0 && x < d_pixelWidth);
      
      if (labelId == d_fieldLabel->id())
      {
        assert(y > d_maxYByX[x]);
        
        d_maxYByX[x] = y;
      }
    }
    
    std::vector<uchar>& getEdgeYValues() const { return d_maxYByX; }

  private:
    std::vector<uchar> d_maxYByX;
    std::shared_ptr<PixelLabel> d_fieldLabel;
    ushort d_pixelWidth;
    ushort d_pixelHeight;
  };
}
