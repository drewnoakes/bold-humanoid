#pragma once

#include <opencv2/core/core.hpp>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../Config/config.hh"
#include "../../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../../HoughLineExtractor/houghlineextractor.hh"
#include "../../PixelLabel/pixellabel.hh"

namespace bold
{
  /**
   * Builds a cv::Mat of labelled pixels, resembling a cartoon. Access after
   * each image pass via mat().
   */
  class CartoonPass : public ImagePassHandler<uchar>
  {
  public:
    /**
     * @param backgroundColour The colour to use for non-labelled pixels. Defaults to black.
     */
    CartoonPass(ushort width, ushort height, std::vector<std::shared_ptr<PixelLabel>> const& labels)
    : d_mat(height, width, CV_8UC3),
      d_backgroundColour(Config::getSetting<Colour::bgr>("round-table.cartoon.background-colour")),
      d_labels(labels)
    {}

    cv::Mat mat() { return d_mat; }

    void onImageStarting(SequentialTimer& timer) override
    {
      d_mat = d_backgroundColour->getValue().toScalar();

      // Do this each frame, as label definitions can change at runtime
      for (std::shared_ptr<PixelLabel> const& label : d_labels)
      {
        d_bgrByLabelId[label->id()] = label->hsvRange().toBgr();
      }
      timer.timeEvent("Clear");
    }

    void onRowStarting(ushort y, Eigen::Vector2i const& granularity) override
    {
      d_ptr = d_mat.ptr<Colour::bgr>(y);
      d_dx = granularity.x();
    }

    void onPixel(uchar value, ushort x, ushort y) override
    {
      if (value != 0)
      {
        *d_ptr = d_bgrByLabelId[value];
      }
      d_ptr += d_dx;
    }

    std::string id() const override
    {
      return std::string("CartoonPass");
    }
  private:
    cv::Mat d_mat;
    Colour::bgr d_bgrByLabelId[8]; // assumes we'll never have more than 7 labels (1-8)
    Setting<bold::Colour::bgr>* d_backgroundColour;
    Colour::bgr* d_ptr;
    int d_dx;
    std::vector<std::shared_ptr<PixelLabel>> d_labels;
  };
}
