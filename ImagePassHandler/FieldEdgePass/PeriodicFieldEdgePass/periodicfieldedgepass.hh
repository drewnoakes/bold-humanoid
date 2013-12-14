#pragma once

#include <memory>
#include <vector>

#include "../fieldedgepass.hh"

namespace bold
{
  class PeriodicFieldEdgePass : public FieldEdgePass
  {
  public:
    PeriodicFieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight, ushort period);

    void onImageStarting() override;
    void onPixel(uchar labelId, ushort x, ushort y) override;

    ushort getEdgeYValue(ushort x) const override;

  private:
    std::vector<ushort> d_maxYByC;
    std::vector<ushort> d_runByC;
    ushort d_period;
  };
}
