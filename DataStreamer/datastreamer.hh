#ifndef BOLD_DATA_STREAMER_HH
#define BOLD_DATA_STREAMER_HH

#include <vector>
#include <string>
#include <libwebsockets.h>
#include <opencv2/opencv.hpp>
#include "../Debugger/debugger.hh"

namespace cv
{
  class Mat;
}

namespace bold
{
  enum Protocol
  {
    HTTP = 0,
    TIMING,
    GAME_STATE,
    AGENT_MODEL,
    CAMERA,
    PROTOCOL_COUNT
  };

  struct CameraSession
  {
    enum
    {
      SEND_CONTROLS,
      SEND_IMG_TAGS,
      SEND_PREFIX,
      SEND_IMAGE
    };

    unsigned state;
    unsigned tagSelection;
  };

  class DataStreamer
  {
  public:
    DataStreamer(int port);

    void init();
    void update();
    void close();

    void streamImage(cv::Mat const& img);

    int callback_http(
      struct libwebsocket_context* context,
      struct libwebsocket* wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void* in,
      size_t len);

    int callback_timing(
      struct libwebsocket_context* context,
      struct libwebsocket* wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void* in,
      size_t len);

    int callback_game_state(
      struct libwebsocket_context* context,
      struct libwebsocket* wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void* in,
      size_t len);

    int callback_agent_model(
      struct libwebsocket_context* context,
      struct libwebsocket* wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void* in,
      size_t len);

    int callback_camera(
      struct libwebsocket_context* context,
      struct libwebsocket* wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void* in,
      size_t len);

  private:
    const int d_port;
    libwebsocket_context* d_context;
    bool d_gameStateUpdated;
    bool d_agentModelUpdated;

    cv::Mat d_img;

  private:
    static struct libwebsocket_protocols d_protocols[];

    static int _callback_http(
      struct libwebsocket_context* context,
      struct libwebsocket* wsi,
      enum libwebsocket_callback_reasons reason,
      void* user,
      void* in,
      size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->
        callback_http(context, wsi, reason, user, in, len);
    }

    static int _callback_timing(
      struct libwebsocket_context *context,
      struct libwebsocket *wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void *in,
      size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->
        callback_timing(context, wsi, reason, user, in, len);
    }

    static int _callback_game_state(
      struct libwebsocket_context *context,
      struct libwebsocket *wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void *in,
      size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->
        callback_game_state(context, wsi, reason, user, in, len);
    }

    static int _callback_agent_model(
      struct libwebsocket_context *context,
      struct libwebsocket *wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void *in,
      size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->
        callback_agent_model(context, wsi, reason, user, in, len);
    }

    static int _callback_camera(
      struct libwebsocket_context *context,
      struct libwebsocket *wsi,
      enum libwebsocket_callback_reasons reason,
      void *user,
      void *in,
      size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->
        callback_camera(context, wsi, reason, user, in, len);
    }

  };
}

#endif
