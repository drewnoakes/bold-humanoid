#pragma once

#include "../imagecodec.hh"

#include <opencv2/core/core.hpp>

struct png_struct_def;

namespace bold
{
  class PngCodec : public ImageCodec
  {
  public:
    PngCodec();

    bool encode(cv::Mat const& image, std::vector<unsigned char>& buffer);

  private:
    static void writeDataToBuf(png_struct_def* png_ptr, uchar* src, size_t size);
  };
}
