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

libwebsocket_protocols DataStreamer::d_protocols[] = {
  // name, callback, per-session-data-size
  { "http-only", DataStreamer::callback_http, 0, NULL, 0 },
  { "timing-protocol", DataStreamer::callback_timing, 0, NULL, 0 },
  { "game-state-protocol", DataStreamer::callback_game_state, 0, NULL, 0 },
  { "agent-model-protocol", DataStreamer::callback_agent_model, 0, NULL, 0 },
  { NULL, NULL, 0, NULL, 0 },
};

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
      const char* path;
      if (in)
      {
        path = (const char*)in;
        if (strcmp(path, "/") == 0)
        {
          path = "/index.html";
        }
        else if (string(path).find("..") != string::npos)
        {
          // protect against ../ attacks
          std::cout << "[DataStreamer::callback_http] invalid request path: " << path << std::endl;
          return 1;
        }

        std::cout << "[DataStreamer::callback_http] requested path: " << path << std::endl;
      }
      else
      {
        path = "/index.html";
      }

      //
      // Determine the MIME type based upon the path extension
      //
      auto extension = string(path).substr(string(path).find_last_of(".") + 1);
      string mimeType = "application/binary";
      if (extension == "html") {
        mimeType = "text/html";
      }
      else if (extension == "js") {
        mimeType = "text/javascript";
      }
      else if (extension == "json") {
        mimeType = "application/json";
      }
      else if (extension == "css") {
        mimeType = "text/css";
      }
      else if (extension == "ico") {
        mimeType = "image/x-icon";
      }

      char buf[256];
      sprintf(buf, "www%s", path);

      if (libwebsockets_serve_http_file(context, wsi, buf, mimeType.c_str()))
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
                    agentModel.lastSubBoardReadTimeMillis,
                    agentModel.lastThinkCycleMillis);

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

    int n = sprintf((char*)p, "%f|%f|%f|%f|%f|%f|%d",
                    agentModel.cm730State.gyro.x(),
                    agentModel.cm730State.gyro.y(),
                    agentModel.cm730State.gyro.z(),
                    agentModel.cm730State.acc.x(),
                    agentModel.cm730State.acc.y(),
                    agentModel.cm730State.acc.z(),
                    agentModel.mx28States[19].presentPosition);

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
  d_gameStateUpdated(false)
{
  std::cout << "[DataStreamer::DataStreamer] creating on TCP port " << port << std::endl;

//   // name, callback, per-session-data-size
//   libwebsocket_protocols p0 = { "http-only", DataStreamer::callback_http, 0, NULL, 0 };
//   d_protocols[Protocol::HTTP] = p0;
//   libwebsocket_protocols p1 = { "timing-protocol", DataStreamer::callback_timing, 0, NULL, 0 };
//   d_protocols[Protocol::TIMING] = p1;
//   libwebsocket_protocols p2 = { "game-state-protocol", DataStreamer::callback_game_state, 0, NULL, 0 };
//   d_protocols[Protocol::GAME_STATE] = p2;
//   libwebsocket_protocols p3 = { "agent-model-protocol", DataStreamer::callback_agent_model, 0, NULL, 0 };
//   d_protocols[Protocol::AGENT_MODEL] = p3;
//
//   libwebsocket_protocols eol = { NULL, NULL, 0, NULL, 0 };
//   d_protocols[Protocol::PROTOCOL_COUNT] = eol;
}

void DataStreamer::init()
{
  std::cout << "[DataStreamer:init] Starting" << std::endl;

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
  else
    std::cout << "[DataStreamer:init] libwebsocket_context created" << std::endl;

  GameState::getInstance().updated.connect([this]{ d_gameStateUpdated = true; });
  AgentModel::getInstance().updated.connect([this]{ d_agentModelUpdated = true; });

  std::cout << "[DataStreamer:init] Done" << std::endl;
}

void DataStreamer::update()
{
  if (d_context == NULL)
    return;

  //
  // Only register for writing to sockets where we have changes to report
  //
  if (d_gameStateUpdated)
  {
    d_gameStateUpdated = false;
    libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::GAME_STATE]);
  }
  if (d_agentModelUpdated)
  {
    d_agentModelUpdated = false;
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
