#pragma once

#include <vector>

#include <Eigen/Core>

typedef unsigned short ushort;
typedef unsigned char uchar;

namespace bold
{
  struct RowLabels
  {
    RowLabels(uchar* begin, uchar* end, ushort y, Eigen::Matrix<uchar,2,1> granularity)
      : imageY(y),
        granularity(granularity),
        d_begin(begin),
        d_end(end)
    {}

    ushort imageY;
    Eigen::Matrix<uchar,2,1> granularity;

    uchar* begin() const { return d_begin; }
    uchar* end() const { return d_end; }

  private:
    uchar* d_begin;
    uchar* d_end;
  };

  class ImageLabelData
  {
  public:
    ImageLabelData(std::vector<uchar>&& labels, std::vector<RowLabels>&& rows, ushort imageWidth)
      : d_labels(move(labels)),
        d_rows(move(rows)),
        d_imageWidth(imageWidth)
    {}

    std::vector<RowLabels> const& getRows() const { return d_rows; }

    ushort getImageWidth() const { return d_imageWidth; }

  private:
    std::vector<uchar> d_labels;
    std::vector<RowLabels> d_rows;
    ushort d_imageWidth;
  };
}
