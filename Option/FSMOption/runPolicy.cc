#include "fsmoption.ih"

OptionList FSMOption::runPolicy()
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
    {
      if (transition->condition())
      {
        d_curState = transition->childState;
        cout << "[FSMOption::runPolicy] Transition to state: " << d_curState->name << endl;
        d_curState->startTime = Clock::getTimestamp();
        testTransition = true;
        if (transition->onFire)
          transition->onFire();
        break;
      }
    }
  }
  while (testTransition);

  cout << "[FSMOption::runPolicy] Final state: " << d_curState->name << endl;

  return d_curState->options;
}
