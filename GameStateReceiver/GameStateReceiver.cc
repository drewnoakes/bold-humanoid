#include "gamestatereceiver.ih"

GameStateReceiver::GameStateReceiver(shared_ptr<Debugger> debugger, shared_ptr<Voice> voice)
  : d_debugger(debugger),
    d_voice(voice),
    d_sendResponseMessages(Config::getSetting<bool>("game-controller.send-response-messages")),
    d_receivedInfoMessageRecently(false)
{
  int port = Config::getStaticValue<int>("game-controller.tcp-port");

  d_socket = make_shared<UDPSocket>();
  d_socket->setBlocking(false);
  d_socket->bind("", port);

  log::info("GameStateReceiver::GameStateReceiver") << "Listening on UDP port " << port;
}
