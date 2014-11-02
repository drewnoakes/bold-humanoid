#pragma once

#include <opencv2/core/core.hpp>

#include "../imagepasshandler.hh"
#include "../../Config/config.hh"
#include "../../SequentialTimer/sequentialtimer.hh"

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
    CartoonPass(ushort width, ushort height)
    : d_mat(height, width, CV_8UC1)
    {}

    cv::Mat mat() { return d_mat; }

    void onImageStarting(SequentialTimer& timer) override
    {
      d_mat = cv::Scalar(0);
      timer.timeEvent("Clear");
    }

    void onRowStarting(ushort y, Eigen::Vector2i const& granularity) override
    {
      d_ptr = d_mat.ptr<uchar>(y);
      d_dx = granularity.x();
    }

    void onPixel(uchar value, ushort x, ushort y) override
    {
      // NOTE Have tested whether it was better to check if value is zero here, but it runs the
      //      pass 0.3ms slower on average with the if-check here, so skip it. The write pattern
      //      here is sequential anyway. If this becomes random access, it may help.
      *d_ptr = value;
      d_ptr += d_dx;
    }

    std::string id() const override
    {
      return "CartoonPass";
    }

  private:
    cv::Mat d_mat;
    uchar* d_ptr;
    int d_dx;
  };
}
