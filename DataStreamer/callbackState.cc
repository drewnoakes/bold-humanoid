#include "datastreamer.ih"

int DataStreamer::callback_state(
  struct libwebsocket_context* /*context*/,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*session*/,
  void* /*in*/,
  size_t /*len*/)
{
  if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
  {
    // If we're being called, then there's some state to send.
    const libwebsocket_protocols* protocol = libwebsockets_get_protocol(wsi);

    for (shared_ptr<StateObject> state : AgentState::getInstance().allStateObjects())
    {
      if (state && strcmp(state->name().c_str(), protocol->name) == 0)
      {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);

        state->writeJson(writer);

        int n = writeJson(wsi, buffer);

        if (n < 0)
        {
          lwsl_err("ERROR %d writing StateObject JSON to socket\n", n);
          return 1;
        }

        return 0;
      }
    }
  }

  return 0;
}
