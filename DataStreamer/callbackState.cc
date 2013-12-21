#include "datastreamer.ih"

int DataStreamer::callback_state(
  libwebsocket_context* context,
  libwebsocket *wsi,
  libwebsocket_callback_reasons reason,
  void* /*session*/,
  void* /*in*/,
  size_t /*len*/)
{
  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    assert(ThreadId::isDataStreamerThread());
    libwebsocket_callback_on_writable(context, wsi);
    return 0;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    assert(ThreadId::isDataStreamerThread());
    
    // If we're being called, then there's some state to send.
    const libwebsocket_protocols* protocol = libwebsockets_get_protocol(wsi);

    for (shared_ptr<StateTracker const> stateTracker : AgentState::getInstance().getTrackers())
    {
      if (stateTracker && strcmp(stateTracker->name().c_str(), protocol->name) == 0)
      {
        shared_ptr<StateObject const> obj = stateTracker->stateBase();

        if (obj)
        {
          StringBuffer buffer;
          Writer<StringBuffer> writer(buffer);

          obj->writeJson(writer);

          // TODO should probably track !lws_send_pipe_choked(wsi) here, and store full buffer on session

          const char* json = buffer.GetString();
          size_t size = buffer.GetSize();

          unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + size + LWS_SEND_BUFFER_POST_PADDING];
          unsigned char* p = buf + LWS_SEND_BUFFER_POST_PADDING;
          memcpy(p, json, size);

          int n = libwebsocket_write(wsi, p, size, LWS_WRITE_TEXT);

          if (n < 0)
          {
            lwsl_err("ERROR %d writing JSON to socket for StateObject: %s\n", n, protocol->name);
            return 1;
          }
        }
        else
        {
          log::error("DataStreamer::callbackState") << "No StateObject set for: " << protocol->name;
        }

        return 0;
      }
    }
    log::error("DataStreamer::callbackState") << "No StateTracker registered for: " << protocol->name;
    return 0;
  }
  default:
  {
    return 0;
  }
  }
}
