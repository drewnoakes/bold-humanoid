#ifndef BOLD_DATA_STREAMER_HH
#define BOLD_DATA_STREAMER_HH

#include <vector>
#include <string>
#include <libwebsockets.h>

#include "../Debugger/debugger.hh"

namespace bold
{
  enum Protocol
  {
    HTTP = 0,
    TIMING,
    GAME_STATE,
    AGENT_MODEL,
    PROTOCOL_COUNT
  };

  class HttpResource
  {
  public:
    std::string path;
    std::string mimeType;

    HttpResource(std::string path, std::string mimeType)
    : path(path),
      mimeType(mimeType)
    {}
  };

  class DataStreamer
  {
  private:
    static DataStreamer* s_instance;
    
    std::vector<HttpResource> d_resources;
    const int d_port;
    libwebsocket_context* d_context;
    libwebsocket_protocols d_protocols[PROTOCOL_COUNT];

    bool d_gameStateChanged;
    bool d_agentModelChanged;

    static int callback_http(
      struct libwebsocket_context* context,
      struct libwebsocket* wsi,
      enum libwebsocket_callback_reasons reason,
      void* user,
      void* in,
      size_t len);

    static int callback_timing(
      struct libwebsocket_context *context,
      struct libwebsocket *wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void *in,
      size_t len);

    static int callback_game_state(
      struct libwebsocket_context *context,
      struct libwebsocket *wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void *in,
      size_t len);

    static int callback_agent_model(
      struct libwebsocket_context *context,
      struct libwebsocket *wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void *in,
      size_t len);

    DataStreamer(int port);

  public:
    static DataStreamer* create(int port = 8080)
    {
      if (s_instance != nullptr)
        throw "Already initialised.";
      s_instance = new DataStreamer(port);
      s_instance->init();
      return s_instance;
    }

    void init();
    void update();
    void close();
  };
}

#endif
