#include "fsmoption.ih"

#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

vector<shared_ptr<Option>> FSMOption::runPolicy(Writer<StringBuffer>& writer)
{
  auto tryTransition = [this](shared_ptr<FSMTransition>& transition)
  {
    // TODO include information about transitions that were not taken
    // TODO pass JSON writer to conditions too, so they can provide debug information
    if (!transition->condition())
      return false;

    if (d_curState)
    {
      log::info(getId())
        << d_curState->name << " >>> " << transition->childState->name
        << " (" << transition->name << ") after "
        << (int)Clock::getMillisSince(d_curState->startTimestamp) << "ms";
    }
    else
    {
      log::info(getId())
        << "nil" << " >>> " << transition->childState->name
        << " (" << transition->name << ") on entry to FSM";
    }

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

    // If the FSM is paused, don't take the transition
    if (!d_paused->getValue())
    {
      setCurrentState(transition->childState);
      transition->onFire();
    }

    return true;
  };

  auto tryWildcardTransition = [this,tryTransition](shared_ptr<FSMTransition>& transition, Writer<StringBuffer>& writer)
  {
    if (transition->childState == d_curState)
      return false;

    if (tryTransition(transition))
    {
      writer.StartObject();
      writer.String("to");
      writer.String(transition->childState->name.c_str());
      writer.String("via");
      writer.String(transition->name.c_str());
      writer.String("wildcard");
      writer.Bool(true);
      writer.EndObject();
      return true;
    }

    return false;
  };

  if (!d_curState)
  {
    // No state yet (after reset).
    // Give wildcard transitions a chance to set the state.
    writer.String("entry-wildcard-transitions");
    writer.StartArray();
    for (auto& transition : d_wildcardTransitions)
    {
      if (tryWildcardTransition(transition, writer))
        break;
    }
    writer.EndArray();

    // If no wildcard transition set the initial state, use the default start state
    if (!d_curState)
      setCurrentState(d_startState);
  }

  log::trace(getId()) << "Current state: " << d_curState->name;

  const int MAX_LOOP_COUNT = 20;

  writer.String("start");
  writer.String(d_curState->name.c_str());

  // Take as many transitions as possible
  writer.String("transitions");
  writer.StartArray();
  // Count the number of transitions made to protect against endless loops
  int loopCount = 0;
  bool transitionMade;
  do
  {
    transitionMade = false;

    for (auto& transition : d_wildcardTransitions)
    {
      if (tryWildcardTransition(transition, writer))
      {
        transitionMade = true;
        break;
      }
    }

    if (!transitionMade)
    {
      for (auto& transition : d_curState->transitions)
      {
        if (tryTransition(transition))
        {
          writer.StartObject();
          writer.String("to");
          writer.String(transition->childState->name.c_str());
          writer.String("via");
          writer.String(transition->name.c_str());
          writer.EndObject();
          transitionMade = true;
          break;
        }
      }
    }

    if (loopCount++ > MAX_LOOP_COUNT)
    {
      log::error(getId()) << "Transition walk loop exceeded maximum number of iterations. Breaking from loop.";
      break;
    }
  }
  while (transitionMade && !d_paused->getValue());

  writer.EndArray();

  static bool isEndless = false;
  if (loopCount > MAX_LOOP_COUNT)
  {
    writer.String("warning");
    writer.String("Max transition count exceeded. Infinite loop?");
    if (!isEndless)
    {
      if (d_voice->queueLength() < 2)
        d_voice->say("Endless loop detected in option tree");
      isEndless = true;
    }
  }
  else
  {
    isEndless = false;
  }

  log::trace(getId()) << "Final state: " << d_curState->name;

  return d_curState->options;
}
