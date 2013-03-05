#include "datastreamer.ih"

void DataStreamer::sendImage(libwebsocket* wsi, CameraSession* session)
{
  if (!session->imgSending)
  {
    // Haven't started sending image yet, prepare it
    cv::Mat img = d_imgStreams[session->streamSelection].img;
    session->imgJpgBuffer = new vector<uchar>();
    cv::imencode(".jpg", img, *(session->imgJpgBuffer));

    // Send off prefix
    size_t jpgSize = session->imgJpgBuffer->size();
  
    ostringstream prefOut;
    prefOut << jpgSize;
    string pref = prefOut.str();
  
    unsigned char prefBuf[LWS_SEND_BUFFER_PRE_PADDING + pref.size() + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &prefBuf[LWS_SEND_BUFFER_PRE_PADDING];
  
    cout << "[DataStreamer::sendImage] Sending prefix: " << pref << endl;

    memcpy(p, pref.c_str(), pref.size());
    int res = libwebsocket_write(wsi, p, pref.size(), LWS_WRITE_TEXT);
    if (res < 0)
    {
      // TODO: make this method return error code
      lwsl_err("ERROR %d writing to socket (prefix)\n", res);
      return;
    }

    session->imgSending = true;
    session->imgBytesSent = 0;
  }
  
  while (!lws_send_pipe_choked(wsi))
  {
    size_t jpgSize = session->imgJpgBuffer->size();
    uchar* jpgP = session->imgJpgBuffer->data() + session->imgBytesSent;

    int tosend = jpgSize - session->imgBytesSent;
    unsigned bufsize = min(4096, tosend);
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + bufsize + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    
    memcpy(p, jpgP, bufsize);
    
    cout << "[DataStreamer::sendImage] Sending: " << bufsize << "/" << tosend << endl;

    int res = libwebsocket_write(wsi, p, bufsize, LWS_WRITE_BINARY);
    if (res < 0)
    {
      // TODO: make this method return error code
      lwsl_err("ERROR %d writing to socket (image) (%d/%d)\n", res, tosend, session->imgJpgBuffer->size());
      return;
    }
    session->imgBytesSent += bufsize;

    if (session->imgBytesSent == jpgSize)
    {
      // Done sending
      session->imgSending = false;
      session->imgBytesSent = 0;
      delete session->imgJpgBuffer;
      break;
    }
  }

  if (session->imgSending)
  {
    cout << "[DataStreamer::sendImage] Choked before able to send whole image" << endl;
    libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::CAMERA]);
  }
}
