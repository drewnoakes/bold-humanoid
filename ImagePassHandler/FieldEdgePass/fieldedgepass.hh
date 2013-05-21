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
      d_maxRowByCol(pixelWidth)
    {}

    void onImageStarting() override
    {
      assert(d_fieldLabel.size() == d_pixelWidth);
      
      for (ushort x = 0; x < d_pixelWidth; x++)
        d_fieldLabel[x] = d_pixelHeight;
    }

    void onPixel(uchar value, ushort x, ushort y) override
    {
      assert(x >=0 && x < d_pixelWidth);
      
      if (value == d_fieldLabel->id())
      {
        // Increment count in bucket
        if (y < d_maxRowByCol[x])
          d_maxRowByCol[x] = y;
      }
    }
    
    std::vector<uchar>& getEdgeYValues() const { return d_maxRowByCol; }

  private:
    std::vector<uchar> d_maxRowByCol;
    std::shared_ptr<PixelLabel> d_fieldLabel;
    ushort d_pixelWidth;
    ushort d_pixelHeight;
  };
}
