#include <sys/socket.h>
#include <netinet/in.h>
#include <memory>

#include "gamestatereceiver.hh"
#include "../StateObject/GameState/gamestate.hh"

using namespace bold;
using namespace std;

#define GAMECONTROLLER_STRUCT_HEADER "RGme"

shared_ptr<GameState> GameStateReceiver::receive()
{
  const int MAX_LENGTH = 4096;
  static char data[MAX_LENGTH];

  // Process incoming game controller messages:
  static sockaddr_in source_addr;
  socklen_t source_addr_len = sizeof(source_addr);

  // Loop until we get the most recent messages
  // TODO this looping may not be a good idea, in case we miss an important message
  //      there are only two a second, so it shouldn't be hard to process all of them
  while (recvfrom(d_socket, data, MAX_LENGTH, 0, (struct sockaddr*) &source_addr, &source_addr_len) > 0)
  {
    // Verify game controller header:
    if (memcmp(data, GAMECONTROLLER_STRUCT_HEADER, sizeof(GAMECONTROLLER_STRUCT_HEADER) - 1) == 0)
    {
      return make_shared<GameState>(data);
    }
    else
    {
      printf("[GameStateReceiver::receive] ignoring message with unexpected header\n");
    }
  }

  return nullptr;
}