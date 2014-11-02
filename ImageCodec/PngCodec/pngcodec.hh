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
    static void onError(png_struct_def* png_ptr, const char* message);
    static void onWarning(png_struct_def* png_ptr, const char* message);

    std::vector<unsigned char*> rowPointers;
  };
}
