#include "gamestatereceiver.ih"

GameStateReceiver::GameStateReceiver(minIni const& ini, shared_ptr<Debugger> debugger)
: d_socket(-1),
  d_receivedAnything(false),
  d_debugger(debugger)
{
  d_port = ini.geti("GameController", "Port", 3838);

  cout << "[GameStateReceiver::receive] Creating socket for UDP port " << d_port << endl;

  // Create an ordinary UDP socket
  d_socket = socket(AF_INET, SOCK_DGRAM, 0);

  if (d_socket < 0)
  {
    cerr << "[GameStateReceiver::receive] Could not open datagram socket. ErrNo=" << errno << endl;
    throw new runtime_error("Could not open datagram socket");
  }

  // Allow multiple sockets to use the same port
  u_int yes = 1;
  if (setsockopt(d_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
  {
    cerr << "[GameStateReceiver::receive] Reusing address failed." << endl;
    throw new runtime_error("Reusing address failed");
  }

  // Set up destination address
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; // IPv4
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(d_port);

  // Bind to the receive address
  if (bind(d_socket, (struct sockaddr*) &addr, sizeof(addr)) < 0)
  {
    cerr << "[GameStateReceiver::receive] Could not bind to port. ErrNo=" << errno << endl;
    throw new runtime_error("Could not bind to port");
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
    cerr << "[GameStateReceiver::receive] Could not set nonblocking mode. ErrNo=" << errno << endl;
    throw new runtime_error("Could not set nonblocking mode");
  }
}
