#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <libwebsockets.h>
#include <opencv2/opencv.hpp>

#include "../StateObject/stateobject.hh"
#include "../Configurable/configurable.hh"

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
    /** Whether an image is ready to be sent to this client. */
    bool imgReady;
    /** Whether an image is currently in the process of being sent. */
    bool imgSending;
    /** If imgSending is true, the encoded JPEG bytes will be here. */
    std::unique_ptr<std::vector<uchar>> imgJpgBuffer;
    /** If imgSending is true, the number of bytes already sent. */
    unsigned imgBytesSent;
  };

  class DataStreamer : public Configurable
  {
  public:
    DataStreamer(std::shared_ptr<Camera> camera, std::shared_ptr<Debugger> debugger);

    void update();
    void close();

    /** Returns true if there is at least one client connected to the camera image protocol. */
    bool hasImageClients() const { return d_cameraSessions.size() != 0; }

    /** Enqueues an image to be sent to connected clients. */
    void streamImage(cv::Mat const& img);

    void registerControls(std::string family, std::vector<Control> controls);

  private:
    void sendImageBytes(libwebsocket* wsi, CameraSession* session);

    void processCommand(std::string json);
    int writeJson(libwebsocket* wsi, rapidjson::StringBuffer const& buffer);

    std::map<std::string,std::map<unsigned, Control>> d_controlsByIdByFamily;

    cv::Mat d_image;

    std::shared_ptr<Camera> d_camera;
    std::shared_ptr<Debugger> d_debugger;

    int d_port;
    libwebsocket_context* d_context;
    libwebsocket_protocols* d_protocols;
    libwebsocket_protocols* d_cameraProtocol;
    std::vector<CameraSession*> d_cameraSessions;
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
