#pragma once

#include <memory>
#include <rapidjson/stringbuffer.h>

#include "../UDPSocket/udpsocket.hh"

namespace bold
{
  class Agent;
  class BehaviourControl;
  class MessageCounter;

  class DrawBridgeComms
  {
  public:
    DrawBridgeComms(Agent* agent, std::shared_ptr<BehaviourControl> behaviourControl, std::shared_ptr<MessageCounter> messageCounter);

    void publish();

  private:
    void buildMessage(rapidjson::StringBuffer& buffer);

    Agent* d_agent;
    std::shared_ptr<BehaviourControl> d_behaviourControl;
    std::shared_ptr<MessageCounter> d_messageCounter;
    std::unique_ptr<UDPSocket> d_socket;
    std::string d_hostName;
  };
}
