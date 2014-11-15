#pragma once

#include <opencv2/core/core.hpp>

#include "../imagepasshandler.hh"

namespace bold
{
  /** Builds a cv::Mat of labelled pixels, resembling a cartoon. */
  class CartoonPass : public ImagePassHandler
  {
  public:
    CartoonPass(ushort width, ushort height)
      : ImagePassHandler("CartoonPass"),
        d_mat(height, width, CV_8UC1)
    {}

    /** Gets the resulting cartoon image. */
    cv::Mat mat() { return d_mat; }

    void process(ImageLabelData const& labelData, SequentialTimer& timer) override;

  private:
    cv::Mat d_mat;
  };
}
