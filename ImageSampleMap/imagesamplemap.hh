#pragma once

#include <functional>
#include <Eigen/Core>

typedef unsigned short ushort;
typedef unsigned char uchar;

namespace bold
{
  class ImageSampleMap
  {
  public:
    ImageSampleMap(std::function<Eigen::Matrix<uchar,2,1>(ushort)> granularityFunction, ushort width, ushort height);

    int getPixelCount() const;

    std::vector<Eigen::Matrix<uchar,2,1>>::const_iterator begin() const { return d_granularities.begin(); }

  private:
    std::vector<Eigen::Matrix<uchar,2,1>> d_granularities;
    ushort d_width;
  };
}