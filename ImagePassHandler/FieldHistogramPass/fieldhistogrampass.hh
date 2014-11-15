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
  class FieldHistogramPass : public ImagePassHandler
  {
  public:
    FieldHistogramPass(std::shared_ptr<PixelLabel> fieldLabel, ushort height)
      : ImagePassHandler("FieldHistogramPass"),
        d_fieldLabelId((uchar)fieldLabel->getID()),
        d_rowWidths(height),
        d_cumulativePixelCounts(height)
    {}

    void process(ImageLabelData const& labelData, SequentialTimer& timer) override
    {
      std::fill(d_rowWidths.begin(), d_rowWidths.end(), (ushort)0);
      std::fill(d_cumulativePixelCounts.begin(), d_cumulativePixelCounts.end(), (uint)0);
      timer.timeEvent("Clear");

      uint cumulativePixelCount = 0;

      for (auto const& row : labelData)
      {
        const ushort y = row.imageY;
        ushort& countForRow = d_rowWidths[y];

        for (auto const& label : row)
        {
          if (label == d_fieldLabelId)
            countForRow++;
        }

        d_rowWidths[y] *= row.granularity.x();
        cumulativePixelCount += d_rowWidths[y] * row.granularity.y();
        d_cumulativePixelCounts[y] = cumulativePixelCount;
      }
      timer.timeEvent("Process Rows");

      d_cumulativePixelCount = cumulativePixelCount;
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
