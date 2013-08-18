#include "actionoption.ih"

double ActionOption::hasTerminated()
{
  return d_runner && d_runner->getState() == MotionScriptRunnerState::Running ? 0.0 : 1.0;
}
