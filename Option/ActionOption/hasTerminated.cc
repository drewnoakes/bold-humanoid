#include "actionoption.ih"

double ActionOption::hasTerminated()
{
  if (!d_started)
    return false;

  auto actionModule = robotis::Action::GetInstance();
  // It could be we're running something else?
  if (actionModule->IsRunning())
    return 0.0;

  d_started = false;
  return 1.0;
}
