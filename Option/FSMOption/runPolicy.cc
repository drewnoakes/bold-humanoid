#include "fsmoption.ih"

#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

vector<shared_ptr<Option>> FSMOption::runPolicy(Writer<StringBuffer>& writer)
{
  log::verbose(getId()) << " ----- Start -----";

  if (!d_curState)
    setCurrentState(d_startState);

  log::verbose(getId()) << "Current state: " << d_curState->name;

  auto tryTransition = [this](shared_ptr<FSMTransition>& transition)
  {
    // TODO include information about transitions that were not taken
    // TODO pass JSON writer to conditions too, so they can provide debug information
    if (!transition->condition())
      return false;

    log::info(getId())
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

    // If the FSM is paused, don't take the transition
    if (!d_paused->getValue())
    {
      setCurrentState(transition->childState);
      transition->onFire();
    }

    return true;
  };

  const int MAX_LOOP_COUNT = 20;

  writer.String("start").String(d_curState->name.c_str());

  writer.String("transitions").StartArray();

  int loopCount = 0;
  bool transitionMade;
  do
  {
    transitionMade = false;

    for (auto& transition : d_wildcardTransitions)
    {
      if (tryTransition(transition))
      {
        writer.StartObject();
        writer.String("to").String(transition->childState->name.c_str());
        writer.String("via").String(transition->name.c_str());
        writer.String("wildcard").Bool(true);
        writer.EndObject();
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
          writer.String("to").String(transition->childState->name.c_str());
          writer.String("via").String(transition->name.c_str());
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

  if (loopCount > MAX_LOOP_COUNT)
    writer.String("warning").String("Max transition count exceeded. Infinite loop?");

  log::verbose(getId()) << "Final state: " << d_curState->name;

  return d_curState->options;
}
