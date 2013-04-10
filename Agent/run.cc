#include "agent.ih"

int Agent::run()
{
  cout << "[Agent::run] Start" << endl;

  if (!init())
  {
    fprintf(stderr, "[Agent::run] Unable to initialise. Exiting.\n");
    return 1;
  }

  // TODO trap signals and exit normally

  while (true)
  {
    think();
  }

  return 0;
}
