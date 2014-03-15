#include "gamestatereceiver.ih"

#include "../UDPSocket/udpsocket.hh"

GameStateReceiver::GameStateReceiver(shared_ptr<Debugger> debugger, Agent* agent)
  : d_debugger(debugger),
    d_observedOpponentTeamNumbers(),
    d_ignoredTeamNumbers(),
    d_sendResponseMessages(Config::getSetting<bool>("game-controller.send-response-messages")),
    d_agent(agent),
    d_receivedAnything(false)
{
  int port = Config::getStaticValue<int>("game-controller.tcp-port");

  d_socket = make_shared<UDPSocket>();
  d_socket->setBlocking(false);
  d_socket->bind("", port);

  log::info("GameStateReceiver::GameStateReceiver") << "Listening on UDP port " << port;
}
