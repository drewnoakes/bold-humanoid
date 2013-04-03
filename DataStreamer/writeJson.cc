#include "datastreamer.ih"

void DataStreamer::writeJson(libwebsocket* wsi, StringBuffer const& buffer)
{
  const char* json = buffer.GetString();
  size_t size = buffer.GetSize();

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + size + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char* p = buf + LWS_SEND_BUFFER_POST_PADDING;
  memcpy(p, json, size);

  /*int res =*/ libwebsocket_write(wsi, p, size, LWS_WRITE_TEXT);
}
