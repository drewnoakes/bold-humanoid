#pragma once

#include <memory>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>

#include <libwebsockets.h>
#include <opencv2/opencv.hpp>
#include <sigc++/signal.h>

#include "../ImageCodec/JpegCodec/jpegcodec.hh"
#include "../ImageCodec/PngCodec/pngcodec.hh"
#include "../Setting/setting.hh"
#include "../StateObject/stateobject.hh"
#include "../util/assert.hh"
#include "../util/websocketbuffer.hh"

namespace cv
{
  class Mat;
}

namespace bold
{
  class Camera;
  class OptionTree;

  enum class ImageEncoding
  {
    PNG,
    JPEG
  };

  struct CameraSession
  {
    CameraSession(libwebsocket_context* context, libwebsocket *wsi);

    ~CameraSession() = default;

    void notifyImageAvailable(cv::Mat const& image, ImageEncoding encoding, std::map<uchar, Colour::bgr> const& palette);

    int write();

    static PngCodec pngCodec;
    static JpegCodec jpegCodec;

  private:
    /** Whether an image is waiting to be encoded. */
    bool d_imgWaiting;
    /** Whether encoded image data is currently being sent. */
    bool d_imgSending;
    /** If d_imgSending is true, the encoded JPEG bytes will be here. */
    std::unique_ptr<std::vector<uchar>> d_imageBytes;
    /** If d_imgSending is true, the number of bytes already sent. */
    unsigned d_bytesSent;

    std::mutex d_imageMutex;
    cv::Mat d_image;
    ImageEncoding d_imageEncoding;
    std::map<uchar, Colour::bgr> d_palette;

    libwebsocket_context* d_context;
    libwebsocket* d_wsi;
  };

  class JsonSession
  {
  public:
    JsonSession(std::string protocolName, libwebsocket* wsi, libwebsocket_context* context);

    ~JsonSession() = default;

    void enqueue(WebSocketBuffer&& buffer, bool suppressLwsNotify = false);

    int write();

  private:
    std::string _protocolName;
    libwebsocket* _wsi;
    libwebsocket_context* _context;
    /** A queue of websocket buffers containing messages to send for this session. */
    std::queue<WebSocketBuffer> _queue;
    /** The number of bytes already sent of the front message in the queue. */
    int _bytesSent;
    unsigned _maxQueueSeen;
    std::string _hostName;
    std::string _ipAddress;
  };

  class DataStreamer
  {
  public:
    DataStreamer(std::shared_ptr<Camera> camera);

    void stop();

    /// Emits signal whenever the first client connects to a protocol, or the last disconnects.
    sigc::signal<void,std::string,bool> hasClientChanged;

    /** Returns true if there is at least one client connected to the camera image protocol. */
    bool hasCameraClients() const { return d_cameraSessions.size() != 0; }

    /** Enqueues an image to be sent to connected clients. */
    void streamImage(cv::Mat const& img, ImageEncoding imageEncoding, std::map<uchar, Colour::bgr> const& palette);

    void setOptionTree(std::shared_ptr<OptionTree> optionTree) { d_optionTree = optionTree; }

  private:
    void writeSettingUpdateJson(SettingBase const* setting, rapidjson::Writer<WebSocketBuffer>& writer);

    void run();

    void processCommand(std::string json, JsonSession* jsonSession);

    void writeControlSyncJson(rapidjson::Writer<WebSocketBuffer>& writer);

    const int d_port;
    const std::shared_ptr<Camera> d_camera;

    libwebsocket_context* d_context;
    libwebsocket_protocols* d_protocols;
    libwebsocket_protocols* d_controlProtocol;

    std::vector<CameraSession*> d_cameraSessions;
    std::mutex d_cameraSessionsMutex;

    std::vector<JsonSession*> d_controlSessions;
    std::mutex d_controlSessionsMutex;

    std::multimap<std::string, JsonSession*> d_stateSessions;
    std::mutex d_stateSessionsMutex;

    bool d_isStopRequested;
    std::thread d_thread;
    std::shared_ptr<OptionTree> d_optionTree;

    //
    // libwebsocket callbacks
    //

    int callback_http   (libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len);
    int callback_camera (libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len);
    int callback_control(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len);
    int callback_state  (libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len);

    static int _callback_camera(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_camera(context, wsi, reason, user, in, len);
    }

    static int _callback_http(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_http(context, wsi, reason, user, in, len);
    }

    static int _callback_control(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_control(context, wsi, reason, user, in, len);
    }

    static int _callback_state(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_state(context, wsi, reason, user, in, len);
    }
  };
}
