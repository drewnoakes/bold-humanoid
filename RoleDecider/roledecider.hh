#pragma once

#include "../StateObject/TeamState/teamstate.hh"

namespace bold
{
  class Agent;
  class Debugger;

  class RoleDecider
  {
  public:
    RoleDecider(std::shared_ptr<Debugger> debugger)
    : d_debugger(debugger)
    {}

    PlayerRole getRole() const { return d_role; }

    void update();

  private:
    std::shared_ptr<Debugger> d_debugger;
    PlayerRole d_role;
  };
}
