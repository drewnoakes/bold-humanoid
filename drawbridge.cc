#include <iostream>
#include <libwebsockets.h>

#include "../Config/config.hh"
#include "../UDPSocket/udpsocket.hh"
#include "../util/log.hh"

using namespace bold;
using namespace std;

struct Session
{
  std::shared_ptr<std::vector<uchar> const> data;
  /** The number of bytes sent from the front message in the queue. */
  unsigned bytesSent;
};

vector<Session*> sessions;

int websocket_callback(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
  Session* session = reinterpret_cast<Session*>(user);

  switch (reason)
  {
    case LWS_CALLBACK_ESTABLISHED:
    {
      log::info("WebSockets") << "Client connected";
      sessions.push_back(session);
      return 0;
    }
    case LWS_CALLBACK_CLOSED:
    {
      log::info("WebSockets") << "Client disconnected";
      auto it = find(sessions.begin(), sessions.end(), session);
      if (it != sessions.end())
        sessions.erase(it);
      return 0;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE:
    {
      // Fill the outbound pipe with frames of data
      while (!lws_send_pipe_choked(wsi) && session->data)
      {
        shared_ptr<vector<uchar> const> const& str = session->data;
        assert(str);
        uint totalSize = str.get()->size();

        assert(session->bytesSent < totalSize);

        const uchar* start = str.get()->data() + session->bytesSent;

        uint remainingSize = totalSize - session->bytesSent;
        uint frameSize = min(2048u, remainingSize);
        uchar frameBuffer[LWS_SEND_BUFFER_PRE_PADDING + frameSize + LWS_SEND_BUFFER_POST_PADDING];
        uchar *p = &frameBuffer[LWS_SEND_BUFFER_PRE_PADDING];

        memcpy(p, start, frameSize);

        int writeMode = session->bytesSent == 0
          ? LWS_WRITE_TEXT
          : LWS_WRITE_CONTINUATION;

        if (frameSize != remainingSize)
          writeMode |= LWS_WRITE_NO_FIN;

        int res = libwebsocket_write(wsi, p, frameSize, (libwebsocket_write_protocol)writeMode);

        if (res < 0)
        {
          lwsl_err("ERROR %d writing to socket (control)\n", res);
          return 1;
        }

        session->bytesSent += frameSize;

        if (session->bytesSent == totalSize)
        {
          // Done sending this queue item, so ditch it, reset and loop around again
          session->data = nullptr;
          session->bytesSent = 0;
          break;
        }
      }

      // Queue for more writing later on if we still have data remaining
      if (session->data)
        libwebsocket_callback_on_writable(context, wsi);

      return 0;
    }
    default:
    {
      return 0;
    }
  }
}

int main()
{
  static constexpr uint MaxMessageSize = 1024*1024;

  log::minLevel = LogLevel::Info;

  Config::initialise("configuration-metadata.json", "configuration-agent.json");

  cout << "I'm alive!!!" << endl;

  int udpPort = Config::getStaticValue<int>("drawbridge.udp-port");
  int wsPort = Config::getStaticValue<int>("drawbridge.websocket-port");

  //
  // UDP socket for listening
  //

  UDPSocket socket;
  socket.setBlocking(false);
  socket.bind("", udpPort);

  //
  // WebSocket for publishing
  //
  libwebsocket_protocols* d_protocols = new libwebsocket_protocols[2];

                   // name, callback, per-session-data-size, rx-buffer-size, no-buffer-all-partial-tx
  d_protocols[0] = { "drawbridge", websocket_callback, sizeof(Session), 0, 0 };
  // Mark the end of the protocols
  d_protocols[1] = { nullptr, nullptr, 0, 0, 0 };

  lws_context_creation_info contextInfo;
  memset(&contextInfo, 0, sizeof(contextInfo));
  contextInfo.port = wsPort;
  contextInfo.protocols = d_protocols;
  contextInfo.gid = contextInfo.uid = -1;
//  contextInfo.user = this;
  libwebsocket_context* d_context = libwebsocket_create_context(&contextInfo);

  if (d_context == nullptr)
  {
    log::error("WebSockets") << "libwebsocket context creation failed";
    exit(1);
  }
  log::info("WebSockets") << "Listening on TCP port " << wsPort;

  while (true)
  {
    //
    // Listen for UDP packet
    //

    static char data[MaxMessageSize];

    int bytesRead = socket.receive(data, MaxMessageSize);

    // Returns zero bytes when no message available (non-blocking)
    // Returns -1 when an error occured. UDPSocket logs the error.
    if (bytesRead > 0)
    {
      //
      // Process WebSocket clients
      //
      auto str = make_shared<vector<uchar>>(bytesRead);
      memcpy(str->data(), data, bytesRead);
      cout << "Received message. " << sessions.size() << " sessions." << endl;
      for (auto const& session : sessions)
      {
        session->data = str;
        session->bytesSent = 0;
      }
      libwebsocket_callback_on_writable_all_protocol(&d_protocols[0]);
    }

    //
    // Process whatever else needs doing on the socket (new clients, etc)
    // This is normally very fast
    //
    libwebsocket_service(d_context, 0);
  }
}
