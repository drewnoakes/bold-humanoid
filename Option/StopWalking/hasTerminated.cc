#include "stopwalking.ih"

double StopWalking::hasTerminated()
{
  return d_ambulator->isRunning() ? 1.0 : 0.0;
}