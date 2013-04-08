#include "fsmoption.ih"

OptionPtr FSMOption::runPolicy()
{
  if (!d_curState)
    d_curState = d_startState;

  bool testTransition = true;
  do
  {
    testTransition = false;
    for (auto transition : d_curState->transitions)
      if (transition->condition())
      {
        d_curState = transition->nextState;
        testTransition = true;
        break;
      }
  }
  while (testTransition);

  return d_curState->option;
}
