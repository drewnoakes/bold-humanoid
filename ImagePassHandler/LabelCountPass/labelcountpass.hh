#pragma once

#include <opencv2/core/core.hpp>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../../HoughLineExtractor/houghlineextractor.hh"
#include "../../PixelLabel/pixellabel.hh"

namespace bold
{
  /**
   * Counts the number of each label value seen in an image.
   */
  class LabelCountPass : public ImagePassHandler<uchar>
  {
  public:

    // assuming we'll never have more than 7 labels (1-8)
    static const uchar MAX_LABEL_COUNT = 8;

    LabelCountPass(std::vector<std::shared_ptr<PixelLabel>> const& labels)
      : d_countByLabelId(),
        d_labels(labels)
    {}

    void onImageStarting() override
    {
      for (std::shared_ptr<PixelLabel> label : d_labels)
      {
        assert(label->id() < MAX_LABEL_COUNT);
        d_countByLabelId[label->id()] = 0;
      }
    }

    void onPixel(uchar value, ushort x, ushort y) override
    {
      if (value != 0)
      {
        d_countByLabelId[value]++;
      }
    }

    std::map<std::shared_ptr<PixelLabel>,uint> getCounts() const
    {
      std::map<std::shared_ptr<PixelLabel>,uint> counts;

      for (std::shared_ptr<PixelLabel> label : d_labels)
      {
        counts[label] = d_countByLabelId[label->id()];
      }

      return counts;
    }

  private:
    uint d_countByLabelId[MAX_LABEL_COUNT];
    std::vector<std::shared_ptr<PixelLabel>> d_labels;
  };
}
