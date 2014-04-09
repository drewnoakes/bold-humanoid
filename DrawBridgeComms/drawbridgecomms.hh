#pragma once

#include <memory>
#include <rapidjson/stringbuffer.h>

#include "../UDPSocket/udpsocket.hh"

namespace bold
{
  class BehaviourControl;
  class Debugger;

  class DrawBridgeComms
  {
  public:
    DrawBridgeComms(std::shared_ptr<BehaviourControl> behaviourControl, std::shared_ptr<Debugger> debugger);

    void publish();

  private:
    void buildMessage(rapidjson::StringBuffer& buffer);

    std::shared_ptr<BehaviourControl> d_behaviourControl;
    std::shared_ptr<Debugger> d_debugger;
    std::unique_ptr<UDPSocket> d_socket;
  };
}
