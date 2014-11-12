#pragma once

#include "../imagepasshandler.hh"
#include "../../SequentialTimer/sequentialtimer.hh"

#include <algorithm>
#include <vector>

namespace bold
{
  /**
   * Builds a histogram of field pixels per image row.
   */
  class FieldHistogramPass : public ImagePassHandler<uchar>
  {
  public:
    FieldHistogramPass(std::shared_ptr<PixelLabel> fieldLabel, ushort height)
      : d_fieldLabelId((uchar)fieldLabel->getID()),
        d_rowWidths(height),
        d_cumulativePixelCounts(height)
    {}

    void onImageStarting(SequentialTimer& timer) override
    {
      d_cumulativePixelCount = 0;
      std::fill(d_rowWidths.begin(), d_rowWidths.end(), (ushort)0);
      std::fill(d_cumulativePixelCounts.begin(), d_cumulativePixelCounts.end(), (uint)0);
      timer.timeEvent("Clear");
    }

    void onPixel(uchar value, ushort x, ushort y) override
    {
      if (value == d_fieldLabelId)
        d_rowWidths[y]++;
    }

    void onRowCompleted(ushort y, Eigen::Matrix<uchar,2,1> const& granularity) override
    {
      d_rowWidths[y] *= granularity.x();
      d_cumulativePixelCount += d_rowWidths[y] * granularity.y();
      d_cumulativePixelCounts[y] = d_cumulativePixelCount;
    }

    std::string id() const override
    {
      return "FieldHistogramPass";
    }

    ushort getRowWidth(ushort y) const
    {
      return d_rowWidths[y];
    }

    double getRatioBeneath(ushort y) const
    {
      return d_cumulativePixelCount == 0
        ? 0.0
        : (double)d_cumulativePixelCounts[y] / d_cumulativePixelCount;
    }

  private:
    const uchar d_fieldLabelId;
    std::vector<ushort> d_rowWidths;
    std::vector<uint> d_cumulativePixelCounts;
    uint d_cumulativePixelCount;
  };
}
