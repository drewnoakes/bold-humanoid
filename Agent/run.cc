#include "agent.ih"

int Agent::run()
{
  cout << "[Agent::run] Start" << endl;

  // TODO trap signals and exit normally

  while (true)
  {
    think();
  }

  return 0;
}
