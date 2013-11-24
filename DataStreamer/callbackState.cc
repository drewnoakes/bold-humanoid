#include "datastreamer.ih"

int DataStreamer::callback_state(
  struct libwebsocket_context* context,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*session*/,
  void* /*in*/,
  size_t /*len*/)
{
  assert(ThreadId::isThinkLoopThread());

  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    libwebsocket_callback_on_writable(context, wsi);
    return 0;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
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

          int n = writeJson(wsi, buffer);

          if (n < 0)
          {
            lwsl_err("ERROR %d writing JSON to socket for StateObject: %s\n", n, protocol->name);
            return 1;
          }
        }
        else
        {
          cerr << ccolor::error << "[DataStreamer::callbackState] No StateObject set for: " << protocol->name << ccolor::reset << endl;
        }

        return 0;
      }
    }
    cerr << ccolor::error << "[DataStreamer::callbackState] No StateTracker registered for: " << protocol->name << ccolor::reset << endl;
    return 0;
  }
  default:
  {
    return 0;
  }
  }
}
