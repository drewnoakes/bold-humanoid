#include "fsmoption.ih"

OptionList FSMOption::runPolicy()
{
  cout << "[FSMOption::runPolicy] ----- Start -----" << endl;

  if (!d_curState)
    d_curState = d_startState;

  cout << "[FSMOption::runPolicy] Current state: " << d_curState->name << endl;

  const int MAX_LOOP_COUNT = 20;

  int loopCount = 0;
  bool transitionMade;
  do
  {
    transitionMade = false;

    for (auto transition : d_curState->transitions)
    {
      if (transition->condition())
      {
        d_curState = transition->childState;
        cout << "[FSMOption::runPolicy] Transition to state: " << d_curState->name << endl;
        d_curState->startTime = Clock::getSeconds();
        transitionMade = true;
        if (transition->onFire)
          transition->onFire();
        break;
      }
    }

    if (loopCount++ > MAX_LOOP_COUNT)
    {
      cerr << "[FSMOption::runPolicy] Transition walk loop exceeded maximum number of iterations. Breaking from loop." << endl;
      break;
    }
  }
  while (transitionMade);

  cout << "[FSMOption::runPolicy] Final state: " << d_curState->name << endl;

  return d_curState->options;
}
