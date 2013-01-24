#include "agent.ih"

void Agent::run()
{
  if (!init())
  {
    fprintf(stderr, "Unable to initialise. Exiting.\n");
    return;
  }

  while (true)
  {
    think();
  }
}
