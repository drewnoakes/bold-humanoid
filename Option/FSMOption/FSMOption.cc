#include "fsmoption.hh"

#include "../../Config/config.hh"
#include "../../Setting/setting-implementations.hh"
#include "../../Voice/voice.hh"

#include <sstream>

using namespace bold;
using namespace std;

FSMOption::FSMOption(shared_ptr<Voice> voice, string const& id)
: Option(id, "FSM"),
  d_voice(voice)
{
  std::stringstream path;
  path << "options.fsms." << id << ".paused";

  std::stringstream desc;
  desc << "Pause " << id;

  d_paused = new BoolSetting(path.str(), false, false, desc.str());

  Config::addSetting(d_paused);
}

void FSMOption::reset()
{
  d_curState = d_startState;
}

shared_ptr<FSMState> FSMOption::getState(string name) const
{
  auto it = std::find_if(
    d_states.begin(),
    d_states.end(),
    [&name](shared_ptr<FSMState> const& state) { return state->name == name; });

  if (it == d_states.end())
    return nullptr;

  return *it;
}

void FSMOption::setCurrentState(shared_ptr<FSMState> state)
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
}
