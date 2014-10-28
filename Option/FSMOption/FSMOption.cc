#include "fsmoption.hh"

#include "../../Config/config.hh"
#include "../../Setting/setting-implementations.hh"
#include "../../Voice/voice.hh"

#include <sstream>

using namespace bold;
using namespace std;
using namespace rapidjson;

FSMOption::FSMOption(shared_ptr<Voice> voice, string const& id)
: Option(id, "FSM"),
  d_voice(voice)
{
  stringstream pausePath;
  pausePath << "options.fsms." << id << ".paused";

  stringstream pauseDesc;
  pauseDesc << "Pause " << id;

  d_paused = new BoolSetting(pausePath.str(), false, pauseDesc.str());
  d_paused->setValue(false);

  Config::addSetting(d_paused);

  stringstream jumpPath;
  jumpPath << "options.fsms." << id << ".goto";

  stringstream jumpDesc;
  jumpDesc << "Set state for " << id << " FSM";

  Config::addAction(jumpPath.str(), jumpDesc.str(), [this](Value* args)
  {
    if (!args || !args->IsObject())
    {
      log::warning("FSMOption::goto") << "Invalid request JSON. Must be an object.";
      return;
    }

    auto stateMember = args->FindMember("state");
    if (stateMember == args->MemberEnd() || !stateMember->value.IsString())
    {
      log::warning("FSMOption::goto") << "Invalid request JSON. Must have a 'state' property of type 'string'.";
      return;
    }
    const char* stateName = stateMember->value.GetString();

    // find state having this name
    auto state = getState(string(stateName));
    if (!state)
    {
      log::warning("FSMOption::goto") << "Invalid request to go to unknown state: " << stateName;
      return;
    }

    setCurrentState(state);
  });
}

void FSMOption::reset()
{
  // Clear the current state. runPolicy will set it to the starting state when next invoked.
  d_curState = nullptr;
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
