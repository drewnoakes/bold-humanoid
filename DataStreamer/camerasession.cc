#include "datastreamer.ih"

using namespace bold;
using namespace rapidjson;
using namespace std;

PngCodec bold::CameraSession::pngCodec;
JpegCodec bold::CameraSession::jpegCodec;

CameraSession::CameraSession(libwebsocket_context* context, libwebsocket *wsi)
  : imgWaiting(false),
    imgSending(false),
    imageBytes(make_unique<vector<uchar>>()),
    d_context(context),
    d_wsi(wsi)
{}

void CameraSession::notifyImageAvailable(cv::Mat const& image, ImageEncoding encoding, std::map<uchar, Colour::bgr> const& palette)
{
  ASSERT(ThreadUtil::isThinkLoopThread());

  {
    lock_guard<mutex> imageGuard(d_imageMutex);
    d_image = image;
    d_imageEncoding = encoding;
    d_palette = palette;
    imgWaiting = true;
  }

  if (!imgSending)
    libwebsocket_callback_on_writable(d_context, d_wsi);
}

int CameraSession::write()
{
  ASSERT(ThreadUtil::isDataStreamerThread());

  if (!imgSending)
  {
    if (!imgWaiting)
      return 0;

    // Take a thread-safe copy of the image to encode
    cv::Mat image;
    ImageEncoding encoding;
    std::map<uchar, Colour::bgr> palette;
    {
      lock_guard<mutex> imageGuard(d_imageMutex);
      image = d_image;
      encoding = d_imageEncoding;
      palette = d_palette;
      imgWaiting = false;
      imgSending = true;
    }

    auto t = Clock::getTimestamp();

    // Encode the image
    ASSERT(imageBytes->size() == 0);
    imageBytes->resize(LWS_SEND_BUFFER_PRE_PADDING);
    switch (encoding)
    {
      case ImageEncoding::PNG:
      {
        if (!pngCodec.encode(image, *imageBytes, palette))
        {
          log::error("CameraSession::write") << "Error encoding image as PNG";
          return 1;
        }
        break;
      }
      case ImageEncoding::JPEG:
      {
        if (!jpegCodec.encode(image, *imageBytes))
        {
          log::error("CameraSession::write") << "Error encoding image as JPEG";
          return 1;
        }
        break;
      }
    }
    imageBytes->resize(imageBytes->size() + LWS_SEND_BUFFER_POST_PADDING);

    log::trace("CameraSession::write") << "Encoded image (" << imageBytes->size() << " bytes) in " << Clock::getMillisSince(t) << " ms";

    bytesSent = 0;
  }

  // Fill the outbound pipe with frames of data
  while (!lws_send_pipe_choked(d_wsi))
  {
    uint totalSize = (uint) imageBytes->size() - LWS_SEND_BUFFER_PRE_PADDING - LWS_SEND_BUFFER_POST_PADDING;

    ASSERT(bytesSent < totalSize);

    uchar* start = imageBytes->data() + LWS_SEND_BUFFER_PRE_PADDING + bytesSent;

    uint remainingSize = totalSize - bytesSent;
    uint frameSize = min(2048u, remainingSize);

    int writeMode = bytesSent == 0
      ? LWS_WRITE_BINARY
      : LWS_WRITE_CONTINUATION;

    if (frameSize != remainingSize)
      writeMode |= LWS_WRITE_NO_FIN;

    bool storePostPadding = bytesSent + frameSize < totalSize;
    std::array<uchar,LWS_SEND_BUFFER_POST_PADDING> postPadding{};
    if (storePostPadding)
      std::copy(start + frameSize, start + frameSize + LWS_SEND_BUFFER_POST_PADDING, postPadding.data());

    int res = libwebsocket_write(d_wsi, start, frameSize, (libwebsocket_write_protocol)writeMode);

    if (res < 0)
    {
      log::error("callback_camera") << "Error " << res << " writing to socket (image)";
      return 1;
    }

    bytesSent += frameSize;

    if (bytesSent == totalSize)
    {
      // Done sending
      imgSending = false;
      bytesSent = 0;
      imageBytes->clear();
      return 0;
    }
    else if (storePostPadding)
    {
      std::copy(postPadding.data(), postPadding.data() + LWS_SEND_BUFFER_POST_PADDING, start + frameSize);
    }
  }

  // Queue for more writing later on if we still have data remaining
  if (imgSending || imgWaiting)
    libwebsocket_callback_on_writable(d_context, d_wsi);

  return 0;
}
