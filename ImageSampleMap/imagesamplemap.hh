#pragma once

#include <functional>
#include <Eigen/Core>

typedef unsigned short ushort;
typedef unsigned char uchar;

namespace bold
{
  /** Produces a map used in sub-sampling image data.
   *
   * A granularity function allows processing fewer than all pixels
   * in the image.
   *
   * Granularity is calculated based on y-value, and is specified in
   * both x and y dimensions.
   */
  class ImageSampleMap
  {
  public:
    ImageSampleMap(std::function<Eigen::Matrix<uchar,2,1>(ushort)> granularityFunction, ushort width, ushort height);

    int getPixelCount() const;

    ushort getSampleRowCount() const { return static_cast<ushort>(d_granularities.size()); }

    std::vector<Eigen::Matrix<uchar,2,1>>::const_iterator begin() const { return d_granularities.begin(); }

  private:
    std::vector<Eigen::Matrix<uchar,2,1>> d_granularities;
    ushort d_width;
  };
}