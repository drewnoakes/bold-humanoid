#ifndef BOLD_DATA_STREAMER_HH
#define BOLD_DATA_STREAMER_HH

#include <vector>
#include <string>
#include <libwebsockets.h>
#include <opencv2/opencv.hpp>

#include "../Debugger/debugger.hh"
#include "../Camera/camera.hh"

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
    /** Whether state and options information has been sent to the client.
     * This should happen immediately upon connection.
     */
    bool hasSentStateAndOptions;
    /** Whether an image is ready to be sent to this client. */
    bool imgReady;
    /** Whether an image is currently in the process of being sent. */
    bool imgSending;
    /** If imgSending is true, the encoded JPEG bytes will be here. */
    std::unique_ptr<std::vector<uchar>> imgJpgBuffer;
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

    // TODO provide port from config via initialise function, rather than in DataStreamer constructor
    void initialise(minIni const& ini);
    void update();
    void close();

    void setCamera(std::shared_ptr<Camera> camera) { d_camera = camera; }

    /** Gets the type of image that clients have requested to view. May be None. */
    ImageType getImageType() const { return d_imageType; }

    /** Gets whether the vision system should provide a debugging image this cycle. */
    bool shouldProvideImage();
    bool drawBlobs() const { return d_drawBlobs; }
    bool drawLineDots() const { return d_drawLineDots; }
    bool drawExpectedLines() const { return d_drawExpectedLines; }
    bool drawObservedLines() const { return d_drawObservedLines; }

    /** Enqueues an image to be sent to connected clients. */
    void streamImage(cv::Mat const& img);

    void registerControls(std::string family, std::vector<Control> controls);

  private:
    void sendCameraControls(libwebsocket* wsi);
    void sendImageBytes(libwebsocket* wsi, CameraSession* session);

    void processCameraCommand(std::string json);
    void writeJson(libwebsocket* wsi, rapidjson::StringBuffer const& buffer);

    std::vector<Control> getDebugControls();

    bool d_gameStateUpdated;
    bool d_agentModelUpdated;

    cv::Mat d_image;
    ImageType d_imageType;
    unsigned d_streamFramePeriod;
    bool d_drawBlobs;
    bool d_drawLineDots;
    bool d_drawExpectedLines;
    bool d_drawObservedLines;

    std::shared_ptr<Camera> d_camera;

    std::vector<CameraSession*> d_cameraSessions;

    std::map<std::string,std::map<unsigned, Control>> d_controlsByIdByFamily;

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
