#include "agent.ih"

int Agent::run()
{
  if (!init())
  {
    fprintf(stderr, "Unable to initialise. Exiting.\n");
    return 1;
  }

  while (true)
  {
    think();
  }

  return 0;
}
