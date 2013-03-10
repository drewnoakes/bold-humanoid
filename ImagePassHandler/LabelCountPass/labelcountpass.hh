#ifndef BOLD_LABEL_COUNT_PASS_HH
#define BOLD_LABEL_COUNT_PASS_HH

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
  private:
    uint d_countByLabelId[8]; // assumes we'll never have more than 7 labels (1-8)
    std::vector<PixelLabel> d_labels;

  public:
    LabelCountPass(std::vector<PixelLabel> labels)
    : d_labels(labels),
      d_countByLabelId()
    {}

    void onImageStarting()
    {
      for (PixelLabel const& label : d_labels)
      {
        d_countByLabelId[label.id()] = 0;
      }
    }

    void onPixel(uchar value, int x, int y)
    {
      if (value != 0)
      {
        d_countByLabelId[value]++;
      }
    }

    std::map<PixelLabel,uint> getCounts()
    {
      std::map<PixelLabel,uint> counts;

      for (PixelLabel const& label : d_labels)
      {
        auto count = d_countByLabelId[label.id()];
        counts[label] = count;
      }

      return counts;
    }
  };
}

#endif