#pragma once

#include <string>
#include <netinet/in.h>

#include "../util/Maybe.hh"

namespace bold
{
  class UDPSocket
  {
  public:
    UDPSocket();
    ~UDPSocket();

    /** Controls whether send/receive operations are blocking or not. */
    bool setBlocking(bool isBlocking);

    /** Controls whether the UDP socket is broadcasting. */
    bool setBroadcast(bool isBroadcast);

    /** Controls whether a sender will receive their own multicast packets.
     *
     * Loopback is enabled for multicast sockets by default.
     */
    bool setMulticastLoopback(bool isLoopback);

    /** Sets the multicast Time-To-Live (TTL).
     *
     * This is the number of hops a packet makes before being discarded.
     * Set to one to keep the datagram on a single subnet.
     */
    bool setMulticastTTL(const u_char ttl);

    /** Sets the target address.
     *
     * @param targetAddress the target address, probably obtained via receiveFrom
     */
    bool setTarget(const sockaddr_in targetAddress);

    /** Sets the target address.
     *
     * @param targetIdAddress an IP address in string form, eg: 123.12.23.34
     * @param port the UDP port number to send to
     */
    bool setTarget(std::string targetIpAddress, int port);

    /** Binds the UDP socket to the specified IP address and port number.
     *
     * @param port the local UDP port number to bind to, or if zero an
     *             ephemeral port will be automatically assigned by the OS
     */
    bool bind(int port);

    /** Sends a message via this UDP socket.
     *
     * If you receive 'permission denied' when attempting to send to a broadcast
     * address, make sure you have called setBroadcast(true) beforehand.
     *
     * @param message the content of the message to be sent, as a std::string
     */
    bool send(std::string const& message);

    /** Sends a message via this UDP socket.
     *
     * If you receive 'permission denied' when attempting to send to a broadcast
     * address, make sure you have called setBroadcast(true) beforehand.
     *
     * @param data the content of the message to be sent as a char*
     * @param dataLength the number of bytes of data to be sent
     */
    bool send(const char* data, int dataLength);

    /** Receive a datagram from the socket's buffer.
     *
     * @returns the number of bytes read, which may be zero if non-blocking, or
     *          negative if an error occurs.
     */
    int receive(char* data, int dataLength);

    /** Receive a datagram from the socket's buffer, and obtain the sender's address.
     *
     * @returns the number of bytes read, which may be zero if non-blocking, or
     *          negative if an error occurs.
     */
    int receiveFrom(char* data, int dataLength, sockaddr_in* fromAddress, int* fromAddressLength);

  private:
    bool resolveIp4Address(std::string const& ip4Address, int port, sockaddr_in* addr);

    int d_socket;
    sockaddr_in* d_target;
    int d_lastSendErrno;
  };
}
