#pragma once

#include "../StateObject/TeamState/teamstate.hh"

namespace bold
{
  class Agent;

  class RoleDecider
  {
  public:
    PlayerRole getRole() const { return d_role; }

    void update();

  private:
    PlayerRole d_role;
  };
}
