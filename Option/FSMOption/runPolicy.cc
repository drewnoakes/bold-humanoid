#include "fsmoption.ih"

#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

vector<shared_ptr<Option>> FSMOption::runPolicy()
{
  log::verbose(getID()) << " ----- Start -----";

  auto setCurrentState = [this](shared_ptr<FSMState> state)
  {
    d_curState = state;
    d_curState->start();

    static Setting<bool>* announceFsmStates = Config::getSetting<bool>("options.announce-fsm-states");
    static Setting<int>* announceRate = Config::getSetting<int>("options.announce-rate-wpm");
    if (announceFsmStates->getValue())
    {
      if (d_voice->queueLength() < 2)
      {
        SpeechTask task = { state->name, (uint)announceRate->getValue(), false };
        d_voice->say(task);
      }
    }
  };

  if (!d_curState)
    setCurrentState(d_startState);

  log::verbose(getID()) << "Current state: " << d_curState->name;

  auto tryTransition = [this,setCurrentState](shared_ptr<FSMTransition> transition)
  {
    if (!transition->condition())
      return false;

    log::info(getID())
      << d_curState->name << "->" << transition->childState->name
      << " (" << transition->name << ") after "
      << (int)Clock::getMillisSince(d_curState->startTimestamp) << "ms";

    static Setting<bool>* announceFsmTransitions = Config::getSetting<bool>("options.announce-fsm-transitions");
    static Setting<int>* announceRate = Config::getSetting<int>("options.announce-rate-wpm");
    if (announceFsmTransitions->getValue() && transition->name.size())
    {
      if (d_voice->queueLength() < 2)
      {
        SpeechTask task = { transition->name, (uint)announceRate->getValue(), false };
        d_voice->say(task);
      }
    }

    setCurrentState(transition->childState);

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
      log::error(getID()) << "Transition walk loop exceeded maximum number of iterations. Breaking from loop.";
      break;
    }
  }
  while (transitionMade);

  log::verbose(getID()) << "Final state: " << d_curState->name;

  return d_curState->options;
}
