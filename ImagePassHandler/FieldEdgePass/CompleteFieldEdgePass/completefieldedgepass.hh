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

    ~CompleteFieldEdgePass() override = default;

    void onImageStarting(SequentialTimer& timer) override;
    void onPixel(uchar labelId, ushort x, ushort y) override;
    void onImageComplete(SequentialTimer& timer) override;

    ushort getEdgeYValue(ushort x) const override;
    std::vector<std::pair<Eigen::Vector2i,Eigen::Vector2i>> getOcclusionRays() const override;

    std::string id() const override
    {
      return std::string("CompleteFieldEdgePass");
    }

  private:
    uchar d_fieldLabelId;
    std::vector<short> d_maxYByX;
    std::vector<short> d_maxYByXConvex;
    std::vector<ushort> d_runByX;
    ushort d_smoothingWindowSize;
  };
}
