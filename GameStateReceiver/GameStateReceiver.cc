#include "gamestatereceiver.ih"

GameStateReceiver::GameStateReceiver(shared_ptr<Debugger> debugger)
  : d_debugger(debugger),
    d_sendResponseMessages(Config::getSetting<bool>("game-controller.send-response-messages")),
{
  int port = Config::getStaticValue<int>("game-controller.tcp-port");

  d_socket = make_shared<UDPSocket>();
  d_socket->setBlocking(false);
  d_socket->bind("", port);

  log::info("GameStateReceiver::GameStateReceiver") << "Listening on UDP port " << port;
}
