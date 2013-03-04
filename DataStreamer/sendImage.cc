#include "datastreamer.ih"

void DataStreamer::sendImage(libwebsocket* wsi, cv::Mat const& img)
{
  // Encode image to jpeg (perhaps do this directly when getting image?)
  vector<uchar> jpgbuf;
  cv::imencode(".jpg", img, jpgbuf);
  
  size_t jpgSize = jpgbuf.size();
  
  // Send size prefix
  ostringstream prefOut;
  prefOut << jpgSize;
  string pref = prefOut.str();
  
  unsigned char prefBuf[LWS_SEND_BUFFER_PRE_PADDING + pref.size() + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &prefBuf[LWS_SEND_BUFFER_PRE_PADDING];
  
  memcpy(p, pref.c_str(), pref.size());
  int res = libwebsocket_write(wsi, p, pref.size(), LWS_WRITE_TEXT);
    if (res < 0)
    {
      // TODO: make this method return error code
      lwsl_err("ERROR %d writing to socket (prefix)\n", res);
      return;
    }
  
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
      // TODO: make this method return error code
      lwsl_err("ERROR %d writing to socket (image)\n", res);
      return;
    }
    tosend -= bufsize;
    jpgP += bufsize;
  }
}
