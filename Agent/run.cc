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
    AgentModel::getInstance().notifyCycleStarting();

    think();
  }

  return 0;
}
