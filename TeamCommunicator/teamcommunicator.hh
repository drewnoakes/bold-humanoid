#pragma once

#include <memory>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

#include "../util/Maybe.hh"

struct sockaddr;

namespace bold
{
  class UDPSocket;
  class Debugger;
  
  class TeamCommunicator
  {
  public:
    TeamCommunicator(std::shared_ptr<Debugger> debugger, int ourTeamNumber, int port);
    
    /** Used in unit testing. */
    void enableLoopback();
    
    bool send(rapidjson::StringBuffer buffer);

    Maybe<rapidjson::Document> tryReceive();
    
  private:
    std::shared_ptr<UDPSocket> d_socket;
    std::shared_ptr<Debugger> d_debugger;
    const int d_ourTeamNumber;
    const int d_port;
  };
}