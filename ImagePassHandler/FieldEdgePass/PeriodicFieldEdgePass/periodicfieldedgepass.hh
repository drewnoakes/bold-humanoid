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

    ~PeriodicFieldEdgePass() override = default;

    void onImageStarting(SequentialTimer& timer) override;
    void onPixel(uchar labelId, ushort x, ushort y) override;
    void onImageComplete(SequentialTimer& timer) override;

    ushort getEdgeYValue(ushort x) const override;
    std::vector<std::pair<Eigen::Vector2i,Eigen::Vector2i>> getOcclusionRays() const override;

    std::string id() const override
    {
      return std::string("PeriodicFieldEdgePass");
    }

  private:
    std::vector<short> d_maxYByC;
    std::vector<short> d_maxYByCConvex;
    std::vector<ushort> d_runByC;
    ushort d_period;
  };
}
