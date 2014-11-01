#include "pngcodec.hh"

#include "../../util/assert.hh"

#include <png.h>

using namespace bold;
using namespace std;

// TODO can compile libpng to support MMX operations (at the expense of thread safety)

// http://www.libpng.org/pub/png/libpng-1.2.5-manual.html

PngCodec::PngCodec()
{}

bool PngCodec::encode(cv::Mat const& image, vector<unsigned char>& buffer)
{
  // TODO may wish to cache and reuse: png_structp and png_infop

  png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (!png_ptr)
    return false;

  png_info* info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr)
  {
    png_destroy_write_struct(&png_ptr, nullptr);
    return false;
  }

  // TODO can avoid setjmp/longjmp (see libpng docs)
  if (setjmp(png_jmpbuf(png_ptr)) != 0)
  {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return false;
  }

  // Set callbacks for custom buffering of output
  png_set_write_fn(png_ptr, &buffer, &bold::PngCodec::writeDataToBuf, nullptr);

  // Set parameters that control the encoding
  png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_FILTER_SUB);
  png_set_compression_level(png_ptr, Z_BEST_SPEED);
  png_set_compression_strategy(png_ptr, Z_RLE);

  ASSERT(image.depth() == CV_8U);
  ASSERT(image.channels() == 3);

  // Set metadata for the IHDR segment (image header)
  png_set_IHDR(
    png_ptr,
    info_ptr,
    (png_uint_32) image.cols,
    (png_uint_32) image.rows,
    8,
    PNG_COLOR_TYPE_RGB,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT);

  // Write all the PNG information before the image
  png_write_info(png_ptr, info_ptr);

  // TODO use palette instead of RGB
//  png_colorp palette;
//  int num_palette;
//  png_set_PLTE(png_ptr, info_ptr, palette, num_palette);

  // Use 1 byte per pixel in 1, 2, or 4-bit depth files
//  png_set_packing(png_ptr);

  // Use blue, green, red order for pixels
  png_set_bgr(png_ptr);

  // Intel is little-endian
  png_set_swap(png_ptr);

  // Prepare pointers required by libpng
  vector<unsigned char*> rowPointers; // TODO convert to field
  rowPointers.resize(image.rows);
  for (int y = 0; y < image.rows; y++)
    rowPointers[y] = image.data + y*image.step;

  // Write the image data
  png_write_image(png_ptr, rowPointers.data());

  // Write the end of file
  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

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