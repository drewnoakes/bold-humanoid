#include "datastreamer.ih"

using namespace bold;
using namespace rapidjson;
using namespace std;

JsonSession::JsonSession(std::string protocolName, libwebsocket* wsi, libwebsocket_context* context)
  : _protocolName(protocolName),
    _wsi(wsi),
    _context(context),
    _queue(),
    _bytesSent(0),
    _maxQueueSeen(0)
{
  // add client host name and IP address
  int fd = libwebsocket_get_socket_fd(wsi);
  char hostName[256];
  char ipAddress[32];
  libwebsockets_get_peer_addresses(context, wsi, fd, hostName, sizeof(hostName), ipAddress, sizeof(ipAddress));
  _hostName = string(hostName);
  _ipAddress = string(ipAddress);
}

int JsonSession::write()
{
  // Fill the outbound pipe with frames of data
  while (!lws_send_pipe_choked(_wsi) && !_queue.empty())
  {
    vector<uchar> const& str = _queue.front();

    uint totalSize = static_cast<uint>(str.size());

    ASSERT(_bytesSent < totalSize);

    const uchar* start = str.data() + _bytesSent;

    uint remainingSize = totalSize - _bytesSent;
    uint frameSize = min(2048u, remainingSize);
    uchar frameBuffer[LWS_SEND_BUFFER_PRE_PADDING + frameSize + LWS_SEND_BUFFER_POST_PADDING];
    uchar *p = &frameBuffer[LWS_SEND_BUFFER_PRE_PADDING];

    memcpy(p, start, frameSize);

    int writeMode = _bytesSent == 0
      ? LWS_WRITE_TEXT
      : LWS_WRITE_CONTINUATION;

    if (frameSize != remainingSize)
      writeMode |= LWS_WRITE_NO_FIN;

    int res = libwebsocket_write(_wsi, p, frameSize, (libwebsocket_write_protocol)writeMode);

    if (res < 0)
    {
      lwsl_err("ERROR %d writing to socket (control)\n", res);
      return 1;
    }

    _bytesSent += frameSize;

    if (_bytesSent == totalSize)
    {
      // Done sending this queue item, so ditch it, reset and loop around again
      _queue.pop();
      _bytesSent = 0;
    }
  }

  // Queue for more writing later on if we still have data remaining
  if (!_queue.empty())
    libwebsocket_callback_on_writable(_context, _wsi);

  return 0;
}

void JsonSession::enqueue(StringBuffer& buffer)
{
  // TODO accept a different implementation of rapidjson's Buffer which builds a vector directly to avoid the copy, or just queue the buffer objects (if moveable)

  ASSERT(buffer.GetSize() != 0);

  auto queueSize = static_cast<unsigned>(_queue.size());

  if (queueSize / 10 > _maxQueueSeen / 10)
  {
    _maxQueueSeen = queueSize;
    log::warning("JsonSession::enqueue") << _protocolName << " max queue seen " << queueSize;
  }

  // If queue is too long, deal with it
  const int MaxQueueSize = 200;
  if (queueSize > MaxQueueSize)
  {
    log::error("StateUpdated") << "JsonSession queue to " << _hostName << '@' << _ipAddress << " for protocol '" << _protocolName << "' too long (" << queueSize << " > " << MaxQueueSize << ") — purging";
    queue<vector<uchar>> empty;
    swap(_queue, empty);
  }
  else
  {
    _queue.emplace(buffer.GetString(), buffer.GetString() + buffer.GetSize());
    libwebsocket_callback_on_writable(_context, _wsi);
  }
}
