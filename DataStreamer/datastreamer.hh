#ifndef BOLD_DATA_STREAMER_HH
#define BOLD_DATA_STREAMER_HH

//#include <stdio.h>
//#include <stdlib.h>
// #include <unistd.h>
// #include <getopt.h>
// #include <string.h>
// #include <sys/time.h>
// #include <assert.h>
// #include <syslog.h>
// #include <signal.h>
#include <vector>
#include <string>

#include <libwebsockets.h>

// #define UNUSED(expr) do { (void)(expr); } while (0)

namespace bold
{
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
    const int d_port;
    libwebsocket_context* d_context;
    libwebsocket_protocols d_protocols[3];
    std::vector<HttpResource> d_resources;

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

  public:
    DataStreamer(int port = 8080);

    void init();
    void update();
    void close();
  };
}

#endif
