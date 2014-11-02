#pragma once

#include "../imagecodec.hh"
#include "../../util/assert.hh"

#include <opencv2/core/core.hpp>

struct png_struct_def;

namespace bold
{
  enum class CompressionStrategy
  {
    Filtered = 1,
    HuffmanOnly = 2,
    RLE = 3,
    Fixed = 4
  };

  class PngCodec : public ImageCodec
  {
  public:
    PngCodec();

    bool encode(cv::Mat const& image, std::vector<unsigned char>& buffer);

    void setCompressionLevel(int level) { ASSERT(level >= 0 && level <= 9); d_compressionLevel = level; }
    void setCompressionStrategy(CompressionStrategy strategy) { d_compressionStrategy = strategy; }

  private:
    static void writeDataToBuf(png_struct_def* png_ptr, uchar* src, size_t size);
    static void onError(png_struct_def* png_ptr, const char* message);
    static void onWarning(png_struct_def* png_ptr, const char* message);

    std::vector<unsigned char*> rowPointers;

    int d_compressionLevel;
    CompressionStrategy d_compressionStrategy;
  };
}
