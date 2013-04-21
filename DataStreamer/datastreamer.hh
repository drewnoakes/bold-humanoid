#ifndef BOLD_DATA_STREAMER_HH
#define BOLD_DATA_STREAMER_HH

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <minIni.h>
#include <libwebsockets.h>
#include <opencv2/opencv.hpp>

#include "../StateObject/stateobject.hh"

namespace cv
{
  class Mat;
}

namespace bold
{
  class Debugger;
  class Camera;
  class Control;

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
    DataStreamer(minIni const& ini, std::shared_ptr<Camera> camera, std::shared_ptr<Debugger> debugger);

    void update();
    void close();

    /** Gets the type of image that clients have requested to view. May be None. */
    ImageType getImageType() const { return d_imageType; }

    /** Gets whether the vision system should provide a debugging image this cycle. */
    bool shouldProvideImage();
    bool shouldDrawBlobs() const { return d_shouldDrawBlobs; }
    bool shouldDrawLineDots() const { return d_shouldDrawLineDots; }
    bool shouldDrawExpectedLines() const { return d_shouldDrawExpectedLines; }
    bool shouldDrawObservedLines() const { return d_shouldDrawObservedLines; }
    bool shouldDrawHorizon() const { return d_shouldDrawHorizon; }

    /** Enqueues an image to be sent to connected clients. */
    void streamImage(cv::Mat const& img);

    void registerControls(std::string family, std::vector<Control> controls);

  private:
    void sendCameraControls(libwebsocket* wsi);
    void sendImageBytes(libwebsocket* wsi, CameraSession* session);

    void processCameraCommand(std::string json);
    int writeJson(libwebsocket* wsi, rapidjson::StringBuffer const& buffer);

    // TODO can this be const?
    std::vector<Control> getDebugControls();
    std::map<std::string,std::map<unsigned, Control>> d_controlsByIdByFamily;

    cv::Mat d_image;
    ImageType d_imageType;
    unsigned d_streamFramePeriod;
    bool d_shouldDrawBlobs;
    bool d_shouldDrawLineDots;
    bool d_shouldDrawExpectedLines;
    bool d_shouldDrawObservedLines;
    bool d_shouldDrawHorizon;

    std::shared_ptr<Camera> d_camera;
    std::shared_ptr<Debugger> d_debugger;

    int d_port;
    libwebsocket_context* d_context;
    libwebsocket_protocols* d_protocols;
    libwebsocket_protocols* d_timingProtocol;
    libwebsocket_protocols* d_cameraProtocol;
    std::vector<CameraSession*> d_cameraSessions;

    //
    // libwebsocket callbacks
    //

    int callback_http  (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_camera(struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_timing(struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_state (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);

    static int _callback_camera(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_camera(context, wsi, reason, user, in, len);
    }

    static int _callback_http(struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_http(context, wsi, reason, user, in, len);
    }

    static int _callback_timing(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_timing(context, wsi, reason, user, in, len);
    }

    static int _callback_state(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_state(context, wsi, reason, user, in, len);
    }
  };
}

#endif
