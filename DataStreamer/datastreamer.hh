#ifndef BOLD_DATA_STREAMER_HH
#define BOLD_DATA_STREAMER_HH

#include <vector>
#include <string>
#include <libwebsockets.h>
#include <opencv2/opencv.hpp>

#include "../Debugger/debugger.hh"
#include "../vision/Camera/camera.hh"

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
    enum class State
    {
      SEND_CONTROLS,
      SEND_IMG_TYPES,
      SEND_IMAGE
    };

    State state;
    /** Whether an image is ready to be sent to this client. */
    bool imgReady;
    /** Whether an image is currently in the process of being sent. */
    bool imgSending;
    /** If imgSending is true, the encoded JPEG bytes will be here. */
    std::vector<uchar>* imgJpgBuffer;
    /** If imgSending is true, the number of bytes already sent. */
    unsigned imgBytesSent;
  };

  enum class ImageType
  {
    None = 0,
    YCbCr = 1,
    RGB = 2,
    Cartoon = 3
  };

  class DataStreamer
  {
  public:
    DataStreamer(int port);

    // TODO provide port from config via initialise function
    void initialise(minIni const& ini);
    void update();
    void close();

    void setCamera(Camera* camera) { d_camera = camera; }

    /** Gets the type of image that clients have requested to view. May be None. */
    ImageType getImageType() const { return d_imageType; }

    bool shouldProvideImage();

    /** Enqueues an image to be sent to connected clients. */
    void streamImage(cv::Mat const& img);

    // TODO this should be configurable
    bool drawLines() const { return true; }
    bool drawBlobs() const { return true; }

  private:
    void sendCameraControls(libwebsocket* wsi);
    void sendImageTypes(libwebsocket* wsi);
    void sendImageBytes(libwebsocket* wsi, CameraSession* session);

    void processCameraCommand(std::string json);

    bool d_gameStateUpdated;
    bool d_agentModelUpdated;

    cv::Mat d_image;
    ImageType d_imageType;
    unsigned d_streamFramePeriod;

    Camera* d_camera;

    std::vector<CameraSession*> d_cameraSessions;

    const int d_port;
    libwebsocket_context* d_context;

    static struct libwebsocket_protocols d_protocols[];

    //
    // libwebsocket callbacks
    //

    int callback_http       (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_timing     (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_game_state (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_agent_model(struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_camera     (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);

    static int _callback_http(struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_http(context, wsi, reason, user, in, len);
    }

    static int _callback_timing(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_timing(context, wsi, reason, user, in, len);
    }

    static int _callback_game_state(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_game_state(context, wsi, reason, user, in, len);
    }

    static int _callback_agent_model(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_agent_model(context, wsi, reason, user, in, len);
    }

    static int _callback_camera(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_camera(context, wsi, reason, user, in, len);
    }
  };
}

#endif
