#include "datastreamer.ih"

int DataStreamer::callback_camera(
  struct libwebsocket_context* /*context*/,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* session,
  void* in,
  size_t len)
{
  CameraSession* cameraSession = reinterpret_cast<CameraSession*>(session);

  switch (reason)
  {
  case LWS_CALLBACK_CLIENT_ESTABLISHED:
  {
    memset(cameraSession, 0, sizeof(CameraSession));

    break;
  }

  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    // Encode image to jpeg (perhaps do this directly when getting image?)
    vector<uchar> jpgbuf;
    cv::imencode(".jpg", d_img, jpgbuf);

    size_t jpgSize = jpgbuf.size();

    // Send size prefix
    ostringstream prefOut;
    prefOut << jpgSize;
    string pref = prefOut.str();

    unsigned char prefBuf[LWS_SEND_BUFFER_PRE_PADDING + pref.size() + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &prefBuf[LWS_SEND_BUFFER_PRE_PADDING];

    memcpy(p, pref.c_str(), pref.size());
    int res = libwebsocket_write(wsi, p, pref.size(), LWS_WRITE_TEXT);

    // Send image
    int tosend = jpgSize;
    unsigned char* jpgP = jpgbuf.data();

    while (tosend > 0)
    {
      unsigned bufsize = min(4096, tosend);
      unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + bufsize + LWS_SEND_BUFFER_POST_PADDING];
      p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
      
      memcpy(p, jpgP, bufsize);
      
      res = libwebsocket_write(wsi, p, bufsize, LWS_WRITE_BINARY);
      if (res < 0)
      {
	lwsl_err("ERROR %d writing to socket\n", res);
	return 1;
      }
      tosend -= bufsize;
      jpgP += bufsize;
    }

    break;
  }

  case LWS_CALLBACK_RECEIVE:
  {
    cout << "[DataStreamer::callback_camera] client receive, len: " << len << endl;
    break;
  }

  }

  return 0;
}

