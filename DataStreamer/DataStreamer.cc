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

#include "../AgentModel/agentmodel.hh"
#include "../GameState/gamestate.hh"

using namespace bold;
using namespace std;

#define UNUSED(expr) do { (void)(expr); } while (0)

/**
 * A singleton is needed, as the C-style function pointers used as callbacks
 * cannot have an object instance.
 */
DataStreamer* DataStreamer::s_instance;

////////////////////////////////////////////////////// http protocol

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
        // TODO replace with std lib 'find' function
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
        if (!found)
        {
          std::cout << "[DataStreamer::callback_http] no suitable resource found for request: " << requestedPath << std::endl;
          // NOTE this will return an empty response, not a 404
          return 1;
        }
      }
      else
      {
        // serve default (last) resource
        serveResource = &s_instance->d_resources[s_instance->d_resources.size() - 1];
      }

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

  // TODO review 512 size here... can apply a better cap
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
  {
    AgentModel& agentModel = AgentModel::getInstance();

    int n = sprintf((char*)p, "%f|%f|%f",
                    agentModel.lastImageCaptureTimeMillis,
                    agentModel.lastImageProcessTimeMillis,
                    agentModel.lastThinkCycleMillis);

    printf("%s\n", p);

    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}

////////////////////////////////////////////////////// game state protocol

int DataStreamer::callback_game_state(
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

  // TODO review 512 size here... can apply a better cap
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
  {
    GameState& gameState = GameState::getInstance();

    int n = sprintf((char*)p, "%d|%d",
                    gameState.secondsRemaining,
                    gameState.playMode);

    printf("%s\n", p);

    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}

////////////////////////////////////////////////////// agent model protocol

int DataStreamer::callback_agent_model(
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

  // TODO review 512 size here... can apply a better cap
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
  {
    AgentModel& agentModel = AgentModel::getInstance();

    int n = sprintf((char*)p, "%f|%f|%f|%f|%f|%f",
                    agentModel.gyroReading.x(),
                    agentModel.gyroReading.y(),
                    agentModel.gyroReading.z(),
                    agentModel.accelerometerReading.x(),
                    agentModel.accelerometerReading.y(),
                    agentModel.accelerometerReading.z());

    printf("%s\n", p);

    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////

DataStreamer::DataStreamer(int port)
: d_port(port),
  d_gameStateChanged(false)
{
  std::cout << "[DataStreamer::DataStreamer] creating on TCP port " << port << std::endl;

  HttpResource resources[] = {
    { "/index.html", "text/html" },
    { "/telemetry.js", "text/javascript" },
    { "/telemetry.css", "text/css" },
    { "/smoothie.js", "text/javascript" },
    { "/jquery-1.9.0.min.js", "text/javascript" },
    { "/BrowserDetect.js", "text/javascript" },
    { "/favicon.ico", "image/x-icon" }
  };
  int resourceCount = sizeof(resources)/sizeof(HttpResource);
  d_resources.assign(resources, resources + resourceCount);

  // name, callback, per-session-data-size
  libwebsocket_protocols p0 = { "http-only", callback_http, 0, NULL, 0 };
  d_protocols[Protocol::HTTP] = p0;
  libwebsocket_protocols p1 = { "timing-protocol", callback_timing, 0, NULL, 0 };
  d_protocols[Protocol::TIMING] = p1;
  libwebsocket_protocols p2 = { "game-state-protocol", callback_game_state, 0, NULL, 0 };
  d_protocols[Protocol::GAME_STATE] = p2;
  libwebsocket_protocols p3 = { "agent-model-protocol", callback_agent_model, 0, NULL, 0 };
  d_protocols[Protocol::AGENT_MODEL] = p3;

  libwebsocket_protocols eol = { NULL, NULL, 0, NULL, 0 };
  d_protocols[Protocol::PROTOCOL_COUNT] = eol;
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

  GameState::getInstance().updated.connect([this]{ d_gameStateChanged = true; });
  AgentModel::getInstance().cm730Updated.connect([this]{ d_agentModelChanged = true; });
}

void DataStreamer::update()
{
  if (d_context == NULL)
    return;

  //
  // Only register for writing to sockets where we have changes to report
  //
  if (d_gameStateChanged)
  {
    d_gameStateChanged = false;
    libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::GAME_STATE]);
  }
  if (d_agentModelChanged)
  {
    d_agentModelChanged = false;
    libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::AGENT_MODEL]);
  }

  //
  // We always have new timing data available
  //
  libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::TIMING]);

  //
  // Process whatever else needs doing on the socket (new clients, etc)
  // This is normally very fast
  //
  libwebsocket_service(d_context, 0);
}

void DataStreamer::close()
{
  if (d_context == NULL)
    return;

  libwebsocket_context_destroy(d_context);
}