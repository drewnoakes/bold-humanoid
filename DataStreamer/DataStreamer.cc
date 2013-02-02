#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <syslog.h>
#include <signal.h>
#include <vector>
#include <iostream>

#include "datastreamer.hh"

using namespace bold;
using namespace std;

#define UNUSED(expr) do { (void)(expr); } while (0)

DataStreamer* DataStreamer::s_instance;

////////////////////////////////////////////////////// http protocol

//    struct serveable
//    {
//      const char *urlpath;
//      const char *mimetype;
//    };
//
//    static constexpr serveable whitelist[] = {
//      { "/favicon.ico", "image/x-icon" },
//      { "/smoothie.js", "text/javascript" },
//      { "/jquery-1.9.0.min.js", "text/javascript" },
//      { "/BrowserDetect.js", "text/javascript" },
//      { "/telemetry.js", "text/javascript" },
//      { "/telemetry.css", "text/css" },
//      // last one is the default served if no match
//      { "/index.html", "text/html" },
//    };

int DataStreamer::callback_http(
  struct libwebsocket_context* context,
  struct libwebsocket* wsi,
  enum libwebsocket_callback_reasons reason,
  void* user,
  void* in,
  size_t len)
{
  UNUSED(user);
  UNUSED(len);

  switch (reason)
  {
    case LWS_CALLBACK_HTTP:
    {
      HttpResource* serveResource;
      if (in)
      {
        const char* requestedPath = (const char*)in;
        if (strcmp(requestedPath, "/") == 0)
          requestedPath = "/index.html";

        std::cout << "[DataStreamer::callback_http] requested path: " << requestedPath << std::endl;
        bool found = false;
        for (HttpResource resource : s_instance->d_resources)
        {
          if (strcmp(requestedPath, resource.path.c_str()) == 0)
          {
            serveResource = &resource;
            found = true;
            break;
          }
        }
        // TODO make this this return a 404 instead of just a black response?
        if (!found)
        {
          std::cout << "[DataStreamer::callback_http] no suitable resource found for request: " << requestedPath << std::endl;
          return 1;
        }
      }
      else
      {
        // serve default (last) resource
        serveResource = &s_instance->d_resources[s_instance->d_resources.size() - 1];
      }

//           // TODO if none found, return 404 instead of last one
//           printf("HTTP request: %s\n", (const char*)in);
//           unsigned int n;
//           for (n = 0; n < (sizeof(whitelist) / sizeof(whitelist[0]) - 1); n++)
//           {
//             if (in && strcmp((const char*)in, whitelist[n].urlpath) == 0)
//               break;
//           }

      char buf[256];
      sprintf(buf, "www%s", serveResource->path.c_str());

      if (libwebsockets_serve_http_file(context, wsi, buf, serveResource->mimeType.c_str()))
        lwsl_err("Failed to send HTTP file\n");

      break;
    }
    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
    {
      // async sending of file completed. kill the connection.
      return 1;
    }
//         default:
//           break;
  }

  return 0;
}

////////////////////////////////////////////////////// timing protocol

int DataStreamer::callback_timing(
  struct libwebsocket_context *context,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void *user,
  void *in,
  size_t len)
{
  UNUSED(context);
  UNUSED(user);
  UNUSED(in);
  UNUSED(len);

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int n = sprintf((char *)p, "%lu|%u",(tv.tv_sec*1000) + tv.tv_usec/1000, rand());
    printf("%s\n", p);
    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}

DataStreamer::DataStreamer(int port)
: d_port(port)
//       d_protocols{
//         /* first protocol must always be HTTP handler */
//         { "http-only", callback_http, 0, NULL, 0 },
//         { "timing-protocol", callback_timing, 0, NULL, 0 },
// //        { "dumb-increment-protocol", callback_dumb_increment, sizeof(struct per_session_data__dumb_increment), NULL, 0 },
// //        { "lws-mirror-protocol", callback_lws_mirror, sizeof(struct per_session_data__lws_mirror), NULL, 0 },
// //        { "game-control-protocol", callback_game_control, 0, NULL, 0 },
//         { NULL, NULL, 0, NULL, 0 } // end of list
//       }
{
  std::cout << "[DataStreamer::DataStreamer] creating on TCP port " << port << std::endl;

  DataStreamer::s_instance = this;

  #define PROTOCOL_HTTP 0
  #define PROTOCOL_TIMING 1

  HttpResource resources[] = {
    { "/favicon.ico", "image/x-icon" },
    { "/smoothie.js", "text/javascript" },
    { "/jquery-1.9.0.min.js", "text/javascript" },
    { "/BrowserDetect.js", "text/javascript" },
    { "/telemetry.js", "text/javascript" },
    { "/telemetry.css", "text/css" },
    // last one is the default served if no match
    { "/index.html", "text/html" },
  };
  int resourceCount = sizeof(resources)/sizeof(HttpResource);
  d_resources.assign(resources, resources + resourceCount);

  std::cout << d_resources.size() << " resources prepared." << std::endl;

//       HttpResource r1 = {"/favicon.ico", "image/x-icon"};
//       d_resources.push_back(r1);
//       d_resources.push_back(HttpResource("/smoothie.js", "text/javascript"));
//       d_resources.push_back(HttpResource("/jquery-1.9.0.min.js", "text/javascript"));
//       d_resources.push_back(HttpResource("/BrowserDetect.js", "text/javascript"));
//       d_resources.push_back(HttpResource("/telemetry.js", "text/javascript"));
//       d_resources.push_back(HttpResource("/telemetry.css", "text/css"));
//       d_resources.push_back(HttpResource("/index.html", "text/html"));

//       // name, callback, per-session-data-size
//       d_protocols = {
//         /* first protocol must always be HTTP handler */
//         { "http-only", callback_http, 0, NULL, 0 },
//         { "timing-protocol", callback_timing, 0, NULL, 0 },
// //        { "dumb-increment-protocol", callback_dumb_increment, sizeof(struct per_session_data__dumb_increment), NULL, 0 },
// //        { "lws-mirror-protocol", callback_lws_mirror, sizeof(struct per_session_data__lws_mirror), NULL, 0 },
// //        { "game-control-protocol", callback_game_control, 0, NULL, 0 },
//         { NULL, NULL, 0, NULL, 0 } // end of list
//       };

//      argument of type
//       int (bold::DataStreamer::)(libwebsocket_context*, libwebsocket*, libwebsocket_callback_reasons, void*, void*, size_t)
//       int                    (*)(libwebsocket_context*, libwebsocket*, libwebsocket_callback_reasons, void*, void*, size_t)

  // name, callback, per-session-data-size
  struct libwebsocket_protocols p0 = { "http-only", callback_http, 0, NULL, 0 };
  d_protocols[0] = p0;
  struct libwebsocket_protocols p1 = { "timing-protocol", callback_timing, 0, NULL, 0 };
  d_protocols[1] = p1;
}

void DataStreamer::init()
{
  d_context = libwebsocket_create_context(
    d_port,
    /* interface */ NULL,
    d_protocols,
    /*extensions*/ NULL,
    /*ssl_cert_filepath*/ NULL,
    /*ssl_private_key_filepath*/ NULL,
    /*ssl_ca_filepath*/ NULL,
    /* gid */ -1,
    /* uid */ -1,
    /*options*/ 0,
    /* user */ NULL);

  if (d_context == NULL)
    lwsl_err("libwebsocket init failed\n");
}

void DataStreamer::update()
{
  if (d_context == NULL)
    return;

//       static unsigned int oldus = 0;
//       struct timeval tv;
//       gettimeofday(&tv, NULL);
//       if (((unsigned int)tv.tv_usec - oldus) > 50000)
//       {
//         libwebsocket_callback_on_writable_all_protocol(&d_protocols[PROTOCOL_DUMB_INCREMENT]);
//         libwebsocket_callback_on_writable_all_protocol(&d_protocols[PROTOCOL_TIMING]);
//         libwebsocket_callback_on_writable_all_protocol(&d_protocols[PROTOCOL_GAME_CONTROL]);
//         oldus = tv.tv_usec;
//       }

  // We always have new timing data available
  libwebsocket_callback_on_writable_all_protocol(&d_protocols[PROTOCOL_TIMING]);

  // Process whatever needs doing
  libwebsocket_service(d_context, 0);
}

void DataStreamer::close()
{
  if (d_context == NULL)
    return;

  libwebsocket_context_destroy(d_context);
}
