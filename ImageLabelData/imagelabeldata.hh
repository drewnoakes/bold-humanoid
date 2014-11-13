#pragma once

#include <vector>

#include <Eigen/Core>

typedef unsigned short ushort;
typedef unsigned char uchar;

namespace bold
{
  struct RowLabels
  {
    RowLabels(std::vector<uchar>&& labels, ushort y, Eigen::Matrix<uchar,2,1> granularity)
      : imageY(y),
        granularity(granularity),
        labels(move(labels))
    {}

    ushort imageY;
    Eigen::Matrix<uchar,2,1> granularity;
    std::vector<uchar> labels;
  };

  class ImageLabelData
  {
  public:
    ImageLabelData(std::vector<RowLabels>&& rows)
      : d_rows(move(rows))
    {}

    std::vector<RowLabels> const& getRows() const { return d_rows; }

  private:
    std::vector<RowLabels> d_rows;
  };
}
