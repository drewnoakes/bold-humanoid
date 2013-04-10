#include "datastreamer.ih"

int DataStreamer::callback_game_state(
  struct libwebsocket_context* /*context*/,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*user*/,
  void* /*in*/,
  size_t /*len*/)
{
  // TODO review 512 size here... can apply a better cap
  // TODO can we reuse this buffer? static local?
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
  {
    auto const& gameState = AgentState::getInstance().game();

    int n = sprintf((char*)p, "%d|%d",
                    gameState->getSecondsRemaining(),
                    gameState->getPlayMode());

    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}
