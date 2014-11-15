#pragma once

#include "../imagepasshandler.hh"

#include <memory>
#include <vector>

typedef unsigned char uchar;
typedef unsigned int uint;

namespace bold
{
  class ImageLabelData;
  class PixelLabel;
  class SequentialTimer;

  /**
   * Counts the number of each label value seen in an image.
   */
  class LabelCountPass : public ImagePassHandler<uchar>
  {
  public:
    // assuming we'll never have more than 7 labels (1-8)
    static const uchar MAX_LABEL_COUNT = 8;

    LabelCountPass(std::vector<std::shared_ptr<PixelLabel>> const& labels);

    void process(ImageLabelData const& labelData, SequentialTimer& timer) override;

  private:
    uint d_countByLabelId[MAX_LABEL_COUNT];
    std::vector<std::shared_ptr<PixelLabel>> d_labels;
  };
}
