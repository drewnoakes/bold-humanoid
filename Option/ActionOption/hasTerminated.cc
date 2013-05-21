#include "actionoption.ih"

double ActionOption::hasTerminated()
{
  if (!d_started)
    return 0.0;

  // It could be we're running something else?
  if (d_actionModule->isRunning())
    return 0.0;

  d_started = false;
  return 1.0;
}
