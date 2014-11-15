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
      : ImagePassHandler("CartoonPass"),
        d_mat(height, width, CV_8UC1)
    {}

    cv::Mat mat() { return d_mat; }

    void process(ImageLabelData const& labelData, SequentialTimer& timer) override
    {
      d_mat = cv::Scalar(0);
      timer.timeEvent("Clear");

      for (auto const& row : labelData)
      {
        uchar* ptr = d_mat.ptr<uchar>(row.imageY);
        auto dx = row.granularity.x();
        for (auto const& label : row)
        {
          // NOTE Have tested whether it was better to check if value is zero here, but it runs the
          //      pass 0.3ms slower on average with the if-check here, so skip it. The write pattern
          //      here is sequential anyway. If this becomes random access, it may help.
          *ptr = label;
          ptr += dx;
        }
      }
      timer.timeEvent("Process Rows");
    }

  private:
    cv::Mat d_mat;
  };
}
