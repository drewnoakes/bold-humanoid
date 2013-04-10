#include "fsmoption.ih"

OptionPtr FSMOption::runPolicy()
{
  cout << "[FSMOption::runPolicy] ----- Start -----" << endl;

  if (!d_curState)
    d_curState = d_startState;

  cout << "[FSMOption::runPolicy] Current state: " << d_curState->name << endl;

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

  cout << "[FSMOption::runPolicy] New state: " << d_curState->name << endl;

  return d_curState->option;
}
