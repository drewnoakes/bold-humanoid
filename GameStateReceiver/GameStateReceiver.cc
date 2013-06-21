#include "gamestatereceiver.ih"

#include "../UDPSocket/udpsocket.hh"

GameStateReceiver::GameStateReceiver(shared_ptr<Debugger> debugger, Agent* agent)
  : Configurable("gamestatereceiver"),
    d_debugger(debugger),
    d_agent(agent),
    d_receivedAnything(false)
{
  int port = getParam("port", 3838);

  cout << "[GameStateReceiver::receive] Creating socket for UDP port " << port << endl;

  d_socket = make_shared<UDPSocket>();
  d_socket->setBlocking(false);
  d_socket->bind("", port);
}
