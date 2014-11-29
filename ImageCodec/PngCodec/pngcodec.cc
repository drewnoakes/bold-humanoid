#include "pngcodec.hh"

#include "../../Colour/colour.hh"
#include "../../util/log.hh"

#include <png.h>
#include <fstream>
#include <iterator>

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
  if (image.depth() == CV_8UC1 && image.channels() == 1)
  {
    if (!colourByNumber)
    {
      log::error("PngCodec::encode") << "Attempted to write a single-channel image without providing a palette";
      return false;
    }
    colourType = PNG_COLOR_TYPE_PALETTE;
  }
  else if (image.depth() == CV_8U && image.channels() == 3)
  {
    if (colourByNumber && colourByNumber->size())
      log::warning("PngCodec::encode") << "Provided a non-empty palette when writing RGB data";
    colourType = PNG_COLOR_TYPE_RGB;
  }
  else
  {
    log::error("PngCodec::encode") << "Unsupported image format with depth=" << image.depth() << " channels=" << image.channels();
    return false;
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

  // Pixel data has BGR order
  png_set_bgr(png_ptr);

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

  if (colourType == PNG_COLOR_TYPE_PALETTE)
  {
    // Set the colour palette to use
    unsigned paletteSize = 0;
    for (auto const& pair : *colourByNumber)
      paletteSize = max(paletteSize, static_cast<unsigned>(pair.first));
    paletteSize++;

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

bool PngCodec::read(string filePath, cv::Mat& mat)
{
  png_struct* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

  if (!png_ptr)
    return false;

  png_info* info_ptr = png_create_info_struct(png_ptr);
  png_info* end_info = png_create_info_struct(png_ptr);

  if (!info_ptr || !end_info)
    return false;

  // Open the file to read from
  auto file = fopen(filePath.c_str(), "rb");

  if (!file)
    return false;

  if (setjmp(png_jmpbuf(png_ptr)) != 0)
  {
    log::error("PngCodec::read") << "Error reading PNG file";
    fclose(file);
    return false;
  }

  png_init_io(png_ptr, file);

  // Read header
  png_uint_32 width, height;
  int bitDepth, colorType;
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, 0, 0, 0);

  ASSERT(bitDepth == 8);
  ASSERT(colorType == PNG_COLOR_TYPE_RGB);

  // Create
  mat.create((int) height, (int) width, CV_8UC3);

  // Intel is little-endian
  png_set_swap(png_ptr);

  // So long as we're only reading the images we've saved, these options shouldn't be needed
  //png_set_strip_alpha(png_ptr);
  //png_set_palette_to_rgb( png_ptr );
  //png_set_interlace_handling(png_ptr);

  // Pixel data has BGR order
  png_set_bgr(png_ptr);

  png_read_update_info(png_ptr, info_ptr);

  vector<uchar*> buffer;
  buffer.resize(height);
  for (unsigned y = 0; y < height; y++)
    buffer[y] = mat.ptr() + y*mat.step;

  png_read_image(png_ptr, buffer.data());
  png_read_end(png_ptr, end_info);

  fclose(file);

  return true;
}
