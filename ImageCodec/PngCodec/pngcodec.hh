#pragma once

#include "../imagecodec.hh"
#include "../../util/assert.hh"

#include <opencv2/core/core.hpp>

struct png_struct_def;

namespace bold
{
  namespace Colour { class bgr; }

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

    bool encode(cv::Mat const& image, std::vector<unsigned char>& buffer, std::map<uchar, Colour::bgr> const* palette);

    void setCompressionLevel(int level) { ASSERT(level >= 0 && level <= 9); d_compressionLevel = level; }
    void setCompressionStrategy(CompressionStrategy strategy) { d_compressionStrategy = strategy; }

    void setFilterSub(bool enabled) { d_filterSub = enabled; }
    void setFilterUp(bool enabled) { d_filterUp = enabled; }
    void setFilterAvg(bool enabled) { d_filterAvg = enabled; }
    void setFilterPaeth(bool enabled) { d_filterPaeth = enabled; }

  private:
    static void writeDataToBuf(png_struct_def* png_ptr, uchar* src, size_t size);
    static void onError(png_struct_def* png_ptr, const char* message);
    static void onWarning(png_struct_def* png_ptr, const char* message);

    std::vector<unsigned char*> rowPointers;

    int d_compressionLevel;
    CompressionStrategy d_compressionStrategy;
    bool d_filterSub;
    bool d_filterUp;
    bool d_filterAvg;
    bool d_filterPaeth;
  };
}
