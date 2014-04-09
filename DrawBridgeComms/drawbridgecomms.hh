#pragma once

#include <memory>
#include <rapidjson/stringbuffer.h>

#include "../UDPSocket/udpsocket.hh"

namespace bold
{
  class Debugger;

  class DrawBridgeComms
  {
  public:
    DrawBridgeComms();

    void publish();

  private:
    void buildMessage(rapidjson::StringBuffer& buffer);

    std::unique_ptr<UDPSocket> d_socket;
  };
}
