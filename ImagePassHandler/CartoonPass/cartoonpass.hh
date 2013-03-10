#ifndef BOLD_CARTOON_PASS_HH
#define BOLD_CARTOON_PASS_HH

#include <opencv2/core/core.hpp>
#include <vector>

#include "../imagepasshandler.hh"
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
  private:
    cv::Mat d_mat;
    Colour::bgr d_bgrByLabelId[8]; // assumes we'll never have more than 7 labels (1-8)
    bold::Colour::bgr d_backgroundColour;
    Colour::bgr* d_ptr;

  public:
    /**
     * @param backgroundColour The colour to use for non-labelled pixels. Defaults to black.
     */
    CartoonPass(int width, int height, std::vector<PixelLabel> labels, Colour::bgr backgroundColour = Colour::bgr(0,0,0))
    : d_mat(height, width, CV_8UC3),
      d_backgroundColour(backgroundColour)
    {
      for (PixelLabel const& label : labels)
      {
        d_bgrByLabelId[label.id()] = label.hsvRange().toBgr();
      }
    }

    cv::Mat& mat() { return d_mat; }

    void onImageStarting()
    {
      d_mat = d_backgroundColour.toScalar();
    }

    void onRowStarting(int y)
    {
      d_ptr = d_mat.ptr<Colour::bgr>(y);
    }

    void onPixel(uchar value, int x, int y)
    {
      if (value != 0)
      {
        *d_ptr = d_bgrByLabelId[value];
      }
      d_ptr++;
    }
  };
}

#endif