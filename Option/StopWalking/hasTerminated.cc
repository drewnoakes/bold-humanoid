#include "stopwalking.ih"

double StopWalking::hasTerminated()
{
  return d_walkModule->isRunning() ? 1.0 : 0.0;
}
