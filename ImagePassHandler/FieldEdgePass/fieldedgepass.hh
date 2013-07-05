#pragma once

#include <memory>
#include <vector>

#include "../imagepasshandler.hh"

namespace bold
{
  class PixelLabel;

  class FieldEdgePass : public ImagePassHandler<uchar>
  {
  public:
    FieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight);

    void onImageStarting() override;

    void onPixel(uchar labelId, ushort x, ushort y) override;

    ushort getEdgeYValue(ushort x) const;

    void smooth(unsigned windowSize);

  private:
    std::shared_ptr<PixelLabel> d_fieldLabel;
    std::vector<ushort> d_maxYByX;
    ushort d_pixelWidth;
    ushort d_pixelHeight;
  };
}
