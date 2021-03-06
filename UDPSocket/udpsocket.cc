#include "udpsocket.hh"

#include "../util/assert.hh"
#include "../util/log.hh"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

using namespace bold;
using namespace std;

UDPSocket::UDPSocket()
: d_lastSendErrno(0)
{
  d_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (d_socket == -1)
  {
    log::error("UDPSocket::UDPSocket") << "Unable to create datagram socket (errono=" << errno << " " << strerror(errno) << ")";
    throw runtime_error("Unable to create datagram socket");
  }

  d_target = new sockaddr_in;
}

UDPSocket::~UDPSocket()
{
  close(d_socket);
  delete d_target;
}

bool UDPSocket::setBlocking(bool isBlocking)
{
  int flags = fcntl(d_socket, F_GETFL, 0);

  if (flags < 0)
  {
    log::error("UDPSocket::setBlocking") << "Error in F_GETFL: " << strerror(errno) << " (" << errno << ")";
    return false;
  }

  if (isBlocking)
    flags &= ~O_NONBLOCK;
  else
    flags |= O_NONBLOCK;

  if (fcntl(d_socket, F_SETFL, flags) == -1)
  {
    log::error("UDPSocket::setBlocking") << "Error in F_SETFL: " << strerror(errno) << " (" << errno << ")";
    return false;
  }

  return true;
}

bool UDPSocket::setBroadcast(bool isBroadcast)
{
  int isBroadcastInt = isBroadcast ? 1 : 0;

  if (setsockopt(d_socket, SOL_SOCKET, SO_BROADCAST, &isBroadcastInt, sizeof(int)))
  {
    log::error("UDPSocket::setBroadcast") << "Error setting socket option SO_BROADCAST to " << isBroadcast << ": " << strerror(errno) << " (" << errno << ")";
    return false;
  }

  return true;
}

bool UDPSocket::setMulticastLoopback(bool isLoopback)
{
  char isLoopbackChar = isLoopback ? 1 : 0;

  if (setsockopt(d_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &isLoopbackChar, sizeof(char)))
  {
    log::error("UDPSocket::setMulticastLoopback") << "Error setting socket option IP_MULTICAST_LOOP to " << isLoopback << ": " << strerror(errno) << " (" << errno << ")";
    return false;
  }

  return true;
}

bool UDPSocket::setMulticastTTL(const u_char ttl)
{
  if (setsockopt(d_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(u_char)) < 0)
  {
    log::error("UDPSocket::setMulticastTTL") << "Unable to set socket option IP_MULTICAST_TTL to " << (int)ttl;
    return false;
  }
  return true;
}

bool UDPSocket::setTarget(const sockaddr_in targetAddress)
{
  return memcpy(d_target, &targetAddress, sizeof(sockaddr_in));
}

bool UDPSocket::setTarget(string targetIpAddress, int port)
{
  return resolveIp4Address(targetIpAddress, port, (sockaddr_in*)d_target);
}

bool UDPSocket::bind(int port)
{
  static const int one = 1;

  sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons((uint16_t)port);

  // Allow other sockets to bind() to this port, unless there is an active
  // listening socket bound to the port already. This gets around 'Address
  // already in use' errors.
  if (setsockopt(d_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(int)) == -1)
  {
    log::error("UDPSocket::bind") << "Unable to set socket option SO_REUSEADDR: " << strerror(errno) << " (" << errno << ")";
    // Continue, despite this error
  }

  if (::bind(d_socket, (sockaddr*)&addr, sizeof(sockaddr_in)) == -1)
  {
    log::error("UDPSocket::bind") << "Unable to bind socket: " << strerror(errno) << " (" << errno << ")";
    return false;
  }

  // Obtain a textual name for the bound address, and print it out
  char boundAddressStr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(addr.sin_addr), boundAddressStr, INET_ADDRSTRLEN);
  log::info("UDPSocket::bind") << "Socket bound to " << boundAddressStr << ":" << port;

  return true;
}

int UDPSocket::receive(char* data, int dataLength)
{
  return receiveFrom(data, dataLength, nullptr, nullptr);
}

int UDPSocket::receiveFrom(char* data, int dataLength, sockaddr_in* fromAddress, int* fromAddressLength)
{
  ASSERT(bool(fromAddress) == bool(fromAddressLength));

  ssize_t bytesRead = recvfrom(d_socket, data, dataLength, 0, (sockaddr*)fromAddress, (socklen_t*)fromAddressLength);

  if (bytesRead < 0)
  {
    if (errno == EAGAIN)
    {
      // Response indicates that no data was available without blocking, so just return 0
      return 0;
    }

    log::error("UDPSocket::receiveFrom") << "Error (" << errno << "): " << strerror(errno);
    return -1;
  }

  ASSERT(!fromAddress || fromAddress->sin_family == AF_INET);

  return bytesRead;
}

bool UDPSocket::send(string const& message)
{
  return send(message.c_str(), message.length());
}

bool UDPSocket::send(const char* data, int dataLength)
{
  ASSERT(dataLength > 0);
  ASSERT(d_target);
  ASSERT(d_target->sin_family == AF_INET);

  ssize_t bytesSent = sendto(d_socket, data, dataLength, 0, (sockaddr*)d_target, sizeof(sockaddr));

  if (bytesSent < 0)
  {
    if (errno != d_lastSendErrno)
    {
      log::error("UDPSocket::send") << "Error (" << errno << "): " << strerror(errno);
      d_lastSendErrno = errno;
    }
  }
  else if (d_lastSendErrno != 0)
  {
    log::error("UDPSocket::send") << "Successfully sent packet after previous error (" << d_lastSendErrno << "): " << strerror(d_lastSendErrno);
    d_lastSendErrno = 0;
  }

  return bytesSent > 0;
}

bool UDPSocket::resolveIp4Address(string const& ip4Address, int port, sockaddr_in* addr)
{
  memset(addr, 0, sizeof(sockaddr_in));

  addr->sin_family = AF_INET;
  addr->sin_port = htons((uint16_t)port);

  // Populate addr->sin_addr
  if (inet_pton(AF_INET, ip4Address.c_str(), &(addr->sin_addr.s_addr)) != 1)
  {
    log::error("UDPSocket::resolveIp4Address") << "Unable to resolve IP4 address string: " << ip4Address;
    return false;
  }

  return true;
}
