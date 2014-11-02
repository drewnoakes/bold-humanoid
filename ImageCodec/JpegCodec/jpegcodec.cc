#include "jpegcodec.hh"

#include "../../util/log.hh"

#include <jpeglib.h>

using namespace bold;
using namespace std;

typedef unsigned char uchar;

JpegCodec::JpegCodec()
: d_quality(90)
{
}

bool JpegCodec::encode(cv::Mat const& image, vector<uchar>& buffer)
{
  // TODO look at reusing some of these structures

  // Create the JPEG object which represents a compression operation
  struct jpeg_compress_struct cinfo;

  // Set the 'client' data, for use in buffer update callbacks
  cinfo.client_data = &buffer;

  // Set up error handling
  {
    struct jpeg_error_mgr err;
    err.error_exit = JpegCodec::onError;
    cinfo.err = jpeg_std_error(&err);
  }

  // Initialize the JPEG compression
  jpeg_create_compress(&cinfo);

  // Configure data destination
  jpeg_destination_mgr dest;
  cinfo.dest = &dest;
  cinfo.dest->init_destination = &JpegCodec::growBuffer;
  cinfo.dest->empty_output_buffer = [](jpeg_compress_struct* cinfo) -> boolean
  {
    bold::JpegCodec::growBuffer(cinfo);
    return true;
  };
  cinfo.dest->term_destination = [](jpeg_compress_struct* cinfo)
  {
    vector<uchar>* v = reinterpret_cast<vector<uchar>*>(cinfo->client_data);
    ASSERT(v->size() >= cinfo->dest->free_in_buffer);
    v->resize(v->size() - cinfo->dest->free_in_buffer);
  };

  unsigned rows = static_cast<unsigned>(image.rows);
  unsigned cols = static_cast<unsigned>(image.cols);

  // Set the four required parameters
  cinfo.image_width = cols;
  cinfo.image_height = rows;
  cinfo.input_components = 3;         // Number of color components per pixel
  cinfo.in_color_space = JCS_EXT_BGR; // Colour space of input image

  // Set defaults for remaining parameters
  jpeg_set_defaults(&cinfo);

  // Specify custom parameters
  jpeg_set_quality(&cinfo, d_quality, /* limit to baseline-JPEG values */ true);

  // Start compression
  // TODO investigate passing 'false' and writing explicit tables manually
  jpeg_start_compress(&cinfo, /* write all tables */ true);

  // Write scan lines
  rowPointers.resize(rows);
  for (unsigned y = 0; y < image.rows; y++)
    rowPointers[y] = image.data + y*image.step;

  jpeg_write_scanlines(&cinfo, rowPointers.data(), rows);

  // Finish image
  jpeg_finish_compress(&cinfo);

  // Clean up
  jpeg_destroy_compress(&cinfo);

  return true;
}

void JpegCodec::onError(j_common_ptr cinfo)
{
  log::error("JpegCodec::onError") << "Error with message code: " << cinfo->err->msg_code;
}

void JpegCodec::growBuffer(jpeg_compress_struct* cinfo)
{
  auto v = reinterpret_cast<vector<uchar>*>(cinfo->client_data);
  int sizeBefore = (int)v->size();
  v->resize(v->size() + BufferSize);
  cinfo->dest->next_output_byte = v->data() + sizeBefore;
  cinfo->dest->free_in_buffer = v->size() - sizeBefore;
};
