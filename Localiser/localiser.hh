#ifndef BOLD_LOCALISER_HH
#define BOLD_LOCALISER_HH

#include "../AgentPosition/agentposition.hh"

namespace bold
{
  class Localiser
  {
  public:
    Localiser()
    : d_pos(-2.7, 0, 0.23, 0)
    {
      // TODO z value from body configuration (reuse code)
    }

    void update() {}

    AgentPosition position() const { return d_pos; }

  private:
    AgentPosition d_pos;
  };
}

#endif