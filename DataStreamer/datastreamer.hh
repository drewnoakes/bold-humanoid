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

#include "../StateObject/stateobject.hh"

namespace cv
{
  class Mat;
}

namespace bold
{
  class Camera;
  class OptionTree;
  class SettingBase;

  struct CameraSession
  {
    /** Whether an image is ready to be sent to this client. */
    bool imgReady;
    /** Whether an image is currently in the process of being sent. */
    bool imgSending;
    /** If imgSending is true, the encoded JPEG bytes will be here. */
    std::unique_ptr<std::vector<uchar>> imgJpgBuffer;
    /** If imgSending is true, the number of bytes already sent. */
    unsigned bytesSent;
  };

  struct JsonSession
  {
    /** A queue of JSON strings to be sent to the client. */
    std::queue<std::shared_ptr<std::vector<uchar> const>> queue;
    /** The number of bytes sent from the front message in the queue. */
    unsigned bytesSent;

    void initialise();

    int write(libwebsocket* wsi, libwebsocket_context* context);

    static std::shared_ptr<std::vector<uchar>> createBytes(rapidjson::StringBuffer const& buffer)
    {
      auto bytes = std::make_shared<std::vector<uchar>>(buffer.GetSize());
      memcpy(bytes.get()->data(), buffer.GetString(), buffer.GetSize());
      return bytes;
    }
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
    void streamImage(cv::Mat const& img, std::string imageEncoding);

    void setOptionTree(std::shared_ptr<OptionTree> optionTree) { d_optionTree = optionTree; }

  private:
    static std::shared_ptr<std::vector<uchar>> prepareSettingUpdateBytes(SettingBase* setting);

    void run();

    void processCommand(std::string json, JsonSession* jsonSession, libwebsocket_context* context, libwebsocket* wsi);

    std::shared_ptr<std::vector<uchar>> prepareControlSyncBytes();

    const int d_port;
    const std::shared_ptr<Camera> d_camera;

    cv::Mat d_image;
    std::string d_imageEncoding;
    libwebsocket_context* d_context;
    libwebsocket_protocols* d_protocols;
    libwebsocket_protocols* d_cameraProtocol;
    libwebsocket_protocols* d_controlProtocol;
    std::vector<CameraSession*> d_cameraSessions;
    std::vector<JsonSession*> d_controlSessions;
    std::multimap<std::string, JsonSession*> d_stateSessions;
    std::mutex d_stateSessionsMutex;
    bool d_isStopRequested;
    std::thread d_thread;
    std::shared_ptr<OptionTree> d_optionTree;

    //
    // libwebsocket callbacks
    //

    int callback_http   (libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_camera (libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_control(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_state  (libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void *user, void* in, size_t len);

    static int _callback_camera(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_camera(context, wsi, reason, user, in, len);
    }

    static int _callback_http(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_http(context, wsi, reason, user, in, len);
    }

    static int _callback_control(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_control(context, wsi, reason, user, in, len);
    }

    static int _callback_state(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_state(context, wsi, reason, user, in, len);
    }
  };
}
