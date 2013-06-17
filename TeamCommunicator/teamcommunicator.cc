#include "teamcommunicator.hh"

#include "../Debugger/debugger.hh"
#include "../UDPSocket/udpsocket.hh"

using namespace bold;
using namespace std;
using namespace rapidjson;

TeamCommunicator::TeamCommunicator(shared_ptr<Debugger> debugger, int ourTeamNumber, int port)
: d_ourTeamNumber(ourTeamNumber),
  d_port(port)
{
  d_socket = make_shared<UDPSocket>();
  d_socket->setBlocking(false);
  d_socket->setBroadcast(true);
  d_socket->setMulticastTTL(1);
  // TODO incorporate team number into broadcast address
  d_socket->setTarget("255.255.255.255", d_port);
  d_socket->bind("", d_port);
}

void TeamCommunicator::enableLoopback()
{
  d_socket->setMulticastLoopback(true);
}

bool TeamCommunicator::send(StringBuffer buffer)
{
  if (d_debugger)
    d_debugger->notifySendingTeamMessage();
  
  // TODO send data, possible not composing document internally
  // - team number
  // - player number
  // - relative position of ball
  // - whether fallen or not
  // - motion dir/turn angle
  
  return d_socket->send(buffer.GetString(), buffer.GetSize());
}

Maybe<Document> TeamCommunicator::tryReceive()
{
  const int BUFFER_SIZE = 4096;
  static char buffer[BUFFER_SIZE];
  if (d_socket->receive(buffer, BUFFER_SIZE) <= 0)
    return Maybe<Document>::empty();
  
  auto doc = make_shared<Document>();  
  doc->Parse<0>(buffer);

  if (d_debugger)
    d_debugger->notifyReceivedTeamMessage();
  
  return doc;
}
