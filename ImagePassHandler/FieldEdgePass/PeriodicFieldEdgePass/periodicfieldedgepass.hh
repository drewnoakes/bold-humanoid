#pragma once

#include <memory>
#include <vector>

#include "../fieldedgepass.hh"

namespace bold
{
  class PeriodicFieldEdgePass : public FieldEdgePass
  {
  public:
    PeriodicFieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, std::shared_ptr<PixelLabel> lineLabel,
                          ushort imageWidth, ushort imageHeight, ushort period);

    ~PeriodicFieldEdgePass() override = default;

    void process(ImageLabelData const& labelData, SequentialTimer& timer) override;

    ushort getEdgeYValue(ushort x) const override;

    std::vector<OcclusionRay<ushort>> getOcclusionRays() const override;

  private:
    uint8_t d_fieldLabelId;
    uint8_t d_lineLabelId;
    std::vector<short> d_maxYByC;
    std::vector<short> d_maxYByCConvex;
    std::vector<ushort> d_runByC;
    ushort d_period;
  };
}
