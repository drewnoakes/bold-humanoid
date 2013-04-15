#include "datastreamer.ih"

int DataStreamer::callback_state(
  struct libwebsocket_context* context,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*session*/,
  void* /*in*/,
  size_t /*len*/)
{
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
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);

        shared_ptr<StateObject const> obj = stateTracker->stateBase();

        if (obj)
        {
          obj->writeJson(writer);

          int n = writeJson(wsi, buffer);

          if (n < 0)
          {
            lwsl_err("ERROR %d writing StateObject JSON to socket\n", n);
            return 1;
          }
        }
        else
        {
          cout << "No state object for: " << stateTracker->name() << endl;
        }

        return 0;
      }
    }
    return 0;
  }
  default:
  {
    return 0;
  }
  }
}
