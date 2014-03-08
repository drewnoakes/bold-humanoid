#include "fsmoption.hh"

#include "../../Config/config.hh"
#include "../../Setting/setting-implementations.hh"

#include <sstream>

using namespace bold;

FSMOption::FSMOption(std::shared_ptr<Voice> voice, std::string const& id)
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
