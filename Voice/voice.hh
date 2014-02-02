#pragma once

#include <initializer_list>
#include <string>

#include "../util/consumerqueuethread.hh"

namespace bold
{
  class Voice
  {
  public:
    Voice(std::string voice = "default");

    void say(std::string message);

    void sayOneOf(std::initializer_list<std::string> messages);

    void stop();

  private:
    void sayCallback(std::string message);

    std::string d_voiceName;
    ConsumerQueueThread<std::string> d_queue;
    bool d_initialised;
  };
}
