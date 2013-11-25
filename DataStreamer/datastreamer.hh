#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>
#include <queue>

#include <libwebsockets.h>
#include <opencv2/opencv.hpp>

#include "../StateObject/stateobject.hh"

namespace cv
{
  class Mat;
}

namespace bold
{
  class Camera;

  struct CameraSession
  {
    /** Whether an image is ready to be sent to this client. */
    bool imgReady;
    /** Whether an image is currently in the process of being sent. */
    bool imgSending;
    /** If imgSending is true, the encoded JPEG bytes will be here. */
    std::unique_ptr<std::vector<uchar>> imgJpgBuffer;
    /** If imgSending is true, the number of bytes already sent. */
    unsigned imgBytesSent;
  };

  struct ControlSession
  {
    /** A queue of JSON strings to be sent to the client. */
    std::queue<std::shared_ptr<std::vector<uchar> const>> queue;
    /** The number of bytes sent from the front message in the queue. */
    unsigned bytesSent;

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

    void update();
    void close();

    /** Returns true if there is at least one client connected to the camera image protocol. */
    bool hasCameraClients() const { return d_cameraSessions.size() != 0; }

    /** Enqueues an image to be sent to connected clients. */
    void streamImage(cv::Mat const& img);

  private:
    void prepareControlSyncBytes(ControlSession* controlSession) const;
    void prepareImageBytes(libwebsocket_context* context, libwebsocket* wsi, CameraSession* session) const;

    void processCommand(std::string json);

    int writeJson(libwebsocket* wsi, rapidjson::StringBuffer const& buffer);

    cv::Mat d_image;

    std::shared_ptr<Camera> d_camera;

    int d_port;
    libwebsocket_context* d_context;
    libwebsocket_protocols* d_protocols;
    libwebsocket_protocols* d_cameraProtocol;
    libwebsocket_protocols* d_controlProtocol;
    std::vector<CameraSession*> d_cameraSessions;
    std::vector<ControlSession*> d_controlSessions;
    bool d_hasWebSockets;

    //
    // libwebsocket callbacks
    //

    int callback_http   (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_camera (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_control(struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);
    int callback_state  (struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void *user, void* in, size_t len);

    static int _callback_camera(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_camera(context, wsi, reason, user, in, len);
    }

    static int _callback_http(struct libwebsocket_context* context, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_http(context, wsi, reason, user, in, len);
    }

    static int _callback_control(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_control(context, wsi, reason, user, in, len);
    }

    static int _callback_state(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
    {
      return static_cast<DataStreamer*>(libwebsocket_context_user(context))->callback_state(context, wsi, reason, user, in, len);
    }
  };
}
