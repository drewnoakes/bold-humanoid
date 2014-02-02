#include "fsmoption.ih"

#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

vector<shared_ptr<Option>> FSMOption::runPolicy()
{
  log::verbose("FSMOption::runPolicy") << " ----- Start -----";

  auto setCurrentState = [this](shared_ptr<FSMState> state)
  {
    d_curState = state;
    d_curState->start();

    static Setting<bool>* announceFsmStates = Config::getSetting<bool>("options.announce-fsm-states");
    if (announceFsmStates->getValue())
    {
      if (d_voice->queueLength() < 2)
        d_voice->say(state->name);
    }
  };

  if (!d_curState)
    setCurrentState(d_startState);

  log::verbose("FSMOption::runPolicy") << "Current state: " << d_curState->name;

  auto tryTransition = [this,setCurrentState](shared_ptr<FSMTransition> transition)
  {
    if (!transition->condition())
      return false;

    log::info("FSMOption::runPolicy")
      << "(" << getID() << ") transitioning from '" << d_curState->name
      << "' to '" << transition->childState->name << "' after "
      << (int)Clock::getMillisSince(d_curState->startTimestamp) << "ms";

    setCurrentState(transition->childState);

    if (transition->onFire)
      transition->onFire();

    return true;
  };

  const int MAX_LOOP_COUNT = 20;

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
      log::error("FSMOption::runPolicy") << "Transition walk loop exceeded maximum number of iterations. Breaking from loop.";
      break;
    }
  }
  while (transitionMade);

  log::verbose("FSMOption::runPolicy") << "Final state: " << d_curState->name;

  return d_curState->options;
}
