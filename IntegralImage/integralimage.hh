#pragma once

#include <type_traits>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>

#include "../util/assert.hh"

namespace bold
{
  typedef unsigned char uchar;

  /** Models a integral image, also known as a summed area table.
   *
   * Used in some kinds of object recognition.
   *
   * See https://en.wikipedia.org/wiki/Integral_image
   */
  class IntegralImage
  {
  public:
    /** Creates an IntegralImage instance from the specified image.
     *
     * Linear time.
     *
     * @param image Must be a CV_8UC1 image (8bpp)
     */
    static IntegralImage create(cv::Mat const& image);

    IntegralImage(cv::Mat data)
    : d_mat(data)
    {
      ASSERT(data.type() == CV_32SC1 && "Image must be of type CV_32SC1");
    }

    /** Lookup the sum of intensities between the min/max coordinates, inclusive.
     *
     * Constant time.
     */
    int getSummedArea(Eigen::Vector2i min, Eigen::Vector2i max) const;

    /** Return the sum of intensities from (0,0) to the specified coordinates, inclusive.
     */
    int at(unsigned x, unsigned y) const { return d_mat.at<int>(y, x); }

  private:
    cv::Mat d_mat;
  };
}
