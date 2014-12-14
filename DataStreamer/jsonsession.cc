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
  lock_guard<mutex> guard(_queueMutex);

  // Fill the outbound pipe with frames of data
  while (!lws_send_pipe_choked(_wsi) && !_queue.empty())
  {
    WebSocketBuffer& buffer = _queue.front();

    const int totalSize = static_cast<int>(buffer.GetSize()) - LWS_SEND_BUFFER_PRE_PADDING - LWS_SEND_BUFFER_POST_PADDING;

    ASSERT(_bytesSent < totalSize);

    uchar* start = buffer.GetBuffer() + LWS_SEND_BUFFER_PRE_PADDING + _bytesSent;

    const int remainingSize = totalSize - _bytesSent;
    const int frameSize = min(2048, remainingSize);

    int writeMode = _bytesSent == 0
      ? LWS_WRITE_TEXT
      : LWS_WRITE_CONTINUATION;

    if (frameSize != remainingSize)
      writeMode |= LWS_WRITE_NO_FIN;

    bool storePostPadding = _bytesSent + frameSize < totalSize;
    std::array<uchar,LWS_SEND_BUFFER_POST_PADDING> postPadding{};
    if (storePostPadding)
      std::copy(start + frameSize, start + frameSize + LWS_SEND_BUFFER_POST_PADDING, postPadding.data());

    int res = libwebsocket_write(_wsi, start, frameSize, (libwebsocket_write_protocol)writeMode);

    if (res < 0)
    {
      log::error("JsonSession::write") << "Error writing JSON to socket (" << res << ")";
      return 1;
    }

    _bytesSent += frameSize;

    if (_bytesSent == totalSize)
    {
      // Done sending this queue item, so ditch it, reset and loop around again
      _queue.pop();
      _bytesSent = 0;
    }
    else if (storePostPadding)
    {
      std::copy(postPadding.data(), postPadding.data() + LWS_SEND_BUFFER_POST_PADDING, start + frameSize);
    }
  }

  // Queue for more writing later on if we still have data remaining
  if (!_queue.empty())
    libwebsocket_callback_on_writable(_context, _wsi);

  return 0;
}

void JsonSession::enqueue(WebSocketBuffer&& buffer, bool suppressLwsNotify)
{
  lock_guard<mutex> guard(_queueMutex);

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
    queue<WebSocketBuffer> empty;
    swap(_queue, empty);
  }
  else
  {
    buffer.Push(LWS_SEND_BUFFER_POST_PADDING);
    _queue.emplace(move(buffer));

    if (!suppressLwsNotify)
      libwebsocket_callback_on_writable(_context, _wsi);
  }
}
