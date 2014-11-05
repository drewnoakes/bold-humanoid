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
        d_rowCounts(height)
    {}

    void onImageStarting(SequentialTimer& timer) override
    {
      std::fill(d_rowCounts.begin(), d_rowCounts.end(), (ushort)0);
      timer.timeEvent("Clear");
    }

    void onRowStarting(ushort y, Eigen::Vector2i const& granularity) override
    {
      d_pixelWidth = static_cast<ushort>(granularity.x());
    }

    void onPixel(uchar value, ushort x, ushort y) override
    {
      if (value == d_fieldLabelId)
        d_rowCounts[y] += d_pixelWidth;
    }

    std::string id() const override
    {
      return "FieldHistogramPass";
    }

    ushort getRowCount(ushort y) const
    {
      return d_rowCounts[y];
    }

  private:
    const uchar d_fieldLabelId;
    std::vector<ushort> d_rowCounts;
    ushort d_pixelWidth;
  };
}
