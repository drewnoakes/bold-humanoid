#include "string.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "gamestatereceiver.hh"

using namespace bold;

bool GameStateReceiver::receive(struct RoboCupGameControlData* const gameControlData)
{
  const int MAX_LENGTH = 4096;
  static char data[MAX_LENGTH];

  if (!d_init)
  {
    printf("[GameStateReceiver::receive] Creating datagram socket...\n");

    // Create an ordinary UDP socket
    d_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (d_socket < 0)
    {
      printf("[GameStateReceiver::receive] Could not open datagram socket (errno=%d)\n", errno);
      return false;
    }

    // Allow multiple sockets to use the same port
    u_int yes = 1;
    if (setsockopt(d_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
      printf("[GameStateReceiver::receive] Reusing address failed\n");
      return false;
    }

    // Set up destination address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("255.255.255.255"); //htonl(INADDR_ANY);
    addr.sin_port = htons(d_port);

    // Bind to the receive address
    if (bind(d_socket, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
      printf("[GameStateReceiver::receive] Could not bind to port %u (d_socket=%d, errno=%d)\n", d_port, d_socket, errno);
      return false;
    }

    // Request that the kernel join a multicast group
//     struct ip_mreq mreq;
//     mreq.imr_multiaddr.s_addr = inet_addr("255.255.255.255");
//     mreq.imr_interface.s_addr = htonl(INADDR_ANY);
//     if (setsockopt(d_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
//     {
//       printf("setsockopt failed");
//       exit(1);
//     }

    // Nonblocking receive
    int flags = fcntl(d_socket, F_GETFL, 0);

    if (flags == -1)
    {
      flags = 0;
    }

    if (fcntl(d_socket, F_SETFL, flags | O_NONBLOCK) < 0)
    {
      printf("[GameStateReceiver::receive] Could not set nonblocking mode\n");
      return false;
    }

    d_init = true;
  }

  // Process incoming game controller messages:
  static sockaddr_in source_addr;
  socklen_t source_addr_len = sizeof(source_addr);
  int len;

  // Loop until we get the most recent messages
  // TODO this looping may not be a good idea, in case we miss an important message
  //      there are only two a second, so it shouldn't be hard to process all of them
  while ((len = recvfrom(d_socket, data, MAX_LENGTH, 0, (struct sockaddr*) &source_addr, &source_addr_len)) > 0)
  {
    // Verify game controller header:
    //struct RoboCupGameControlData gameControlData;
    if (memcmp(data, GAMECONTROLLER_STRUCT_HEADER, sizeof(GAMECONTROLLER_STRUCT_HEADER) - 1) == 0)
    {
      memcpy(gameControlData, data, sizeof(RoboCupGameControlData));
      return true;
    }
  }

  return false;
}