#pragma once

#include <memory>
#include <vector>

#include "../fieldedgepass.hh"

namespace bold
{
  class CompleteFieldEdgePass : public FieldEdgePass
  {
  public:
    CompleteFieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight);

    void onImageStarting() override;
    void onPixel(uchar labelId, ushort x, ushort y) override;
    void onImageComplete() override;

    ushort getEdgeYValue(ushort x) const override;

  private:
    std::vector<ushort> d_maxYByX;
    std::vector<ushort> d_runByX;
    ushort d_smoothingWindowSize;
  };
}
