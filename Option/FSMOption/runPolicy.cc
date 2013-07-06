#include "fsmoption.ih"

std::vector<std::shared_ptr<Option>> FSMOption::runPolicy()
{
//   cout << "[FSMOption::runPolicy] ----- Start -----" << endl;

  auto setCurrentState = [this](FSMStatePtr state)
  {
    d_curState = state;
    d_curState->start();
  };

  if (!d_curState)
    setCurrentState(d_startState);

//   cout << "[FSMOption::runPolicy] Current state: " << d_curState->name << endl;

  const int MAX_LOOP_COUNT = 20;

  auto tryTransition = [this,setCurrentState](FSMTransitionPtr transition)
  {
    if (!transition->condition())
      return false;

    cout << "[FSMOption::runPolicy] (" << getID() << ") transitioning from '" << d_curState->name
          << "' to '" << transition->childState->name << "' after "
          << (int)((Clock::getSeconds() - d_curState->startTimeSeconds)*1000) << "ms"
          <<  endl;

    setCurrentState(transition->childState);

    if (transition->onFire)
      transition->onFire();

    return true;
  };

  int loopCount = 0;
  bool transitionMade;
  do
  {
    transitionMade = false;

    for (auto transition : d_wildcardTransitions)
    {
      if (tryTransition(transition))
      {
        transitionMade = true;
        break;
      }
    }

    if (!transitionMade)
    {
      for (auto transition : d_curState->transitions)
      {
        if (tryTransition(transition))
        {
          transitionMade = true;
          break;
        }
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
