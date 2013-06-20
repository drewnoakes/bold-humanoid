#pragma once

#include <string>

namespace bold
{
  class Voice
  {
  public:
    Voice(std::string voice = "default");

    void say(const std::string message);

  private:
    std::string d_voiceName;
  };
}
