#include "fsmoption.ih"

std::vector<std::shared_ptr<Option>> FSMOption::runPolicy()
{
//   cout << "[FSMOption::runPolicy] ----- Start -----" << endl;

  if (!d_curState)
  {
    d_curState = d_startState;
    if (d_curState->onEnter)
      d_curState->onEnter();
  }

//   cout << "[FSMOption::runPolicy] Current state: " << d_curState->name << endl;

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
        cout << "[FSMOption::runPolicy] Transition from '" << d_curState->name
             << "' to '" << transition->childState->name << "' after "
             << (int)((Clock::getSeconds() - d_curState->startTimeSeconds)*1000) << "ms"
             <<  endl;

        d_curState = transition->childState;
        d_curState->startTimeSeconds = Clock::getSeconds();

        transitionMade = true;

        if (transition->onFire)
          transition->onFire();

        if (d_curState->onEnter)
          d_curState->onEnter();

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

//   cout << "[FSMOption::runPolicy] Final state: " << d_curState->name << endl;

  return d_curState->options;
}
