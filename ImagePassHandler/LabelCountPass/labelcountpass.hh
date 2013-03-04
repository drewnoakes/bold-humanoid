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
    std::map<uchar,uint> d_countByLabelId;
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
        // TODO would an array via pointer arithmetic be much faster?
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