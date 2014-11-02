#pragma once

#include "../imagecodec.hh"
#include "../../util/assert.hh"
#include "../../Colour/colour.hh"

#include <opencv2/core/core.hpp>

struct jpeg_compress_struct;
struct jpeg_common_struct;

namespace bold
{
  class JpegCodec : public ImageCodec
  {
  public:
    JpegCodec();

    bool encode(cv::Mat const& image, std::vector<unsigned char>& buffer);

    void setQualityLevel(int quality) { ASSERT(quality >= 0 && quality <= 100); d_quality = quality; }

  private:
    static constexpr unsigned BufferSize = 4096 * 5;

    static void onError(jpeg_common_struct* cinfo);
    static void growBuffer(jpeg_compress_struct* cinfo);

    std::vector<unsigned char*> rowPointers;
    int d_quality;
  };
}
