#include "agent.ih"

void Agent::run()
{
  init();

  while (true)
  {
    think();
  }
}
