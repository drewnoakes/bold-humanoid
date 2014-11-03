#include "pngcodec.hh"

#include "../../Colour/colour.hh"
#include "../../util/log.hh"

#include <png.h>

using namespace bold;
using namespace std;

// http://www.libpng.org/pub/png/libpng-1.2.5-manual.html

// TODO may wish to cache and reuse: png_structp and png_infop
// TODO pull out common code from 'encode' overloads
// TODO can compile libpng to support MMX operations (at the expense of thread safety)

PngCodec::PngCodec()
  : d_compressionLevel(-1), // default
    d_compressionStrategy(CompressionStrategy::RLE),
    d_filterSub(true),
    d_filterUp(false),
    d_filterAvg(false),
    d_filterPaeth(false)
{}

bool PngCodec::encode(cv::Mat const& image, vector<unsigned char>& buffer, std::map<uchar, Colour::bgr> const* colourByNumber)
{
  int colourType;
  if (colourByNumber)
  {
    ASSERT(image.depth() == CV_8U);
    ASSERT(image.channels() == 1);
    colourType = PNG_COLOR_TYPE_PALETTE;
  }
  else
  {
    ASSERT(image.depth() == CV_8U);
    ASSERT(image.channels() == 3);
    colourType = PNG_COLOR_TYPE_RGB;
  }

  png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (!png_ptr)
  {
    log::error("PngCodec::PngCodec") << "Error creating libpng png_struct";
    return false;
  }

  png_info* info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr)
  {
    log::error("PngCodec::PngCodec") << "Error creating libpng png_info";
    png_destroy_write_struct(&png_ptr, nullptr);
    return false;
  }

  png_set_error_fn(
    png_ptr,
    png_get_error_ptr(png_ptr),
    &bold::PngCodec::onError,
    &bold::PngCodec::onWarning);

  // Intel is little-endian
  png_set_swap(png_ptr);

  // TODO can avoid setjmp/longjmp (from libpng docs):
  //
  //    If you would rather avoid the complexity of setjmp/longjmp issues, you can compile libpng
  //    with PNG_SETJMP_NOT_SUPPORTED, in which case errors will result in a call to PNG_ABORT()
  //    which defaults to abort().
  //
  if (setjmp(png_jmpbuf(png_ptr)) != 0)
  {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return false;
  }

  // Set parameters that control the encoding
  int filters = PNG_NO_FILTERS;
  if (d_filterSub) filters |= PNG_FILTER_SUB;
  if (d_filterUp) filters |= PNG_FILTER_UP;
  if (d_filterPaeth) filters |= PNG_FILTER_PAETH;
  if (d_filterAvg) filters |= PNG_FILTER_AVG;
  png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, filters);
  png_set_compression_level(png_ptr, d_compressionLevel);
  png_set_compression_strategy(png_ptr, static_cast<int>(d_compressionStrategy));

  // Set metadata for the IHDR segment (image header)
  png_set_IHDR(
    png_ptr,
    info_ptr,
    (png_uint_32) image.cols,
    (png_uint_32) image.rows,
    8, // TODO experiment with lower bit depth for palette images where possible
    colourType,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT);

  // Set callbacks for custom buffering of output
  png_set_write_fn(png_ptr, &buffer, &bold::PngCodec::writeDataToBuf, nullptr);

  png_color* palette = nullptr;

  if (colourByNumber)
  {
    // Set the colour palette to use
    uchar paletteSize = 0;
    for (auto const& pair : *colourByNumber)
      paletteSize = max(paletteSize, pair.first);
    paletteSize++;
    ASSERT(paletteSize <= PNG_MAX_PALETTE_LENGTH);
    palette = (png_color*) png_malloc(png_ptr, paletteSize * sizeof(png_color));
    for (uchar p = 0; p < paletteSize; p++)
    {
      auto const& bgr = colourByNumber->at(p);
      png_color& col = palette[p];
      col.red = bgr.r;
      col.green = bgr.g;
      col.blue = bgr.b;
    }

    png_set_PLTE(png_ptr, info_ptr, palette, paletteSize);
  }

  // Write all the PNG information before the image
  png_write_info(png_ptr, info_ptr);

  // Use 1 byte per pixel in 1, 2, or 4-bit depth files
  png_set_packing(png_ptr);

  // Intel is little-endian
  png_set_swap(png_ptr);

  // Prepare pointers required by libpng
  rowPointers.resize(image.rows);
  for (int y = 0; y < image.rows; y++)
    rowPointers[y] = image.data + y*image.step;

  // Write the image data
  png_write_image(png_ptr, rowPointers.data());

  // Write the end of file
  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  if (palette)
    png_free(png_ptr, palette);

  return true;
}

void PngCodec::writeDataToBuf(png_struct* png_ptr, uchar* src, size_t size)
{
  if (size == 0)
    return;

  vector<unsigned char>* buffer = reinterpret_cast<vector<unsigned char>*>(png_get_io_ptr((png_structp)png_ptr));

  size_t currentSize = buffer->size();

  buffer->resize(currentSize + size);

  std::copy(src, src + size, buffer->begin() + currentSize);
}

void PngCodec::onError(png_struct* png_ptr, const char* message)
{
  log::error("PngCodec::onError") << message;
}

void PngCodec::onWarning(png_struct* png_ptr, const char* message)
{
  log::error("PngCodec::onWarning") << message;
}
