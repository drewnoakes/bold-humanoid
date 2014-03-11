#include "fsmoption.hh"

#include "../../Config/config.hh"
#include "../../Setting/setting-implementations.hh"

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
