#include "datastreamer.ih"

using namespace bold;
using namespace rapidjson;
using namespace std;

CameraSession::CameraSession(libwebsocket_context* context, libwebsocket *wsi)
  : imgReady(false),
    imgSending(false),
    imageBytes(make_unique<vector<uchar>>()),
    d_context(context),
    d_wsi(wsi)
{}

void CameraSession::notifyImageAvailable(cv::Mat const& image, std::string encoding)
{
  ASSERT(ThreadUtil::isThinkLoopThread());

  {
    lock_guard<mutex> imageGuard(d_imageMutex);
    d_image = image;
    d_imageEncoding = encoding;
    imgReady = true;
  }

  if (!imgSending)
    libwebsocket_callback_on_writable(d_context, d_wsi);
}

int CameraSession::write()
{
  ASSERT(ThreadUtil::isDataStreamerThread());

  // WaitingForImage
  // Sending

  if (!imgReady)
    return 0;

  if (!imgSending)
  {
    // Take a thread-safe copy of the image to encode
    cv::Mat image;
    string encoding;
    {
      lock_guard<mutex> imageGuard(d_imageMutex);
      image = d_image;
      encoding = d_imageEncoding;
      imgReady = false;
      imgSending = true;
    }

    // Encode the image
    cv::imencode(encoding, image, *imageBytes);

    bytesSent = 0;
  }

  // Fill the outbound pipe with frames of data
  while (!lws_send_pipe_choked(d_wsi))
  {
    uint totalSize = imageBytes->size();
    uchar* start = imageBytes->data() + bytesSent;

    uint remainingSize = totalSize - bytesSent;
    uint frameSize = min(2048u, remainingSize);
    uchar buf[LWS_SEND_BUFFER_PRE_PADDING + frameSize + LWS_SEND_BUFFER_POST_PADDING];
    uchar *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

    // TODO avoid this copy here by juggling the post padding
    memcpy(p, start, frameSize);

    int writeMode = bytesSent == 0
      ? LWS_WRITE_BINARY
      : LWS_WRITE_CONTINUATION;

    if (frameSize != remainingSize)
      writeMode |= LWS_WRITE_NO_FIN;

    int res = libwebsocket_write(d_wsi, p, frameSize, (libwebsocket_write_protocol)writeMode);

    if (res < 0)
    {
      log::error("callback_camera") << "Error " << res << " writing to socket (image)";
      return 1;
    }

    bytesSent += frameSize;

    if (bytesSent == totalSize)
    {
      // Done sending
      imgReady = false;
      imgSending = false;
      bytesSent = 0;
      return 0;
    }
  }

  // Queue for more writing later on if we still have data remaining
  if (imgSending)
    libwebsocket_callback_on_writable(d_context, d_wsi);

  return 0;
}
