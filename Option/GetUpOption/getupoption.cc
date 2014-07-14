#include "getupoption.hh"

#include "../../Agent/agent.hh"
#include "../../StateObserver/FallDetector/falldetector.hh"
#include "../MotionScriptOption/motionscriptoption.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

GetUpOption::GetUpOption(const string& id, Agent* agent)
: Option(id, "GetUp"),
  d_agent(agent)
{
  auto const& motionScriptModule = agent->getMotionScriptModule();

  d_forwardGetUp     = make_shared<MotionScriptOption>("forward-get-up-script",     motionScriptModule, "./motionscripts/get-up-from-front.json");
  d_backwardGetUp    = make_shared<MotionScriptOption>("backward-get-up-script",    motionScriptModule, "./motionscripts/get-up-from-back.json");
  d_rollLeftToFront  = make_shared<MotionScriptOption>("roll-left-to-front-script",  motionScriptModule, "./motionscripts/roll-left-to-front.json");
  d_rollRightToFront = make_shared<MotionScriptOption>("roll-right-to-front-script", motionScriptModule, "./motionscripts/roll-right-to-front.json");
}

vector<shared_ptr<Option>> GetUpOption::runPolicy(Writer<StringBuffer>& writer)
{
  auto fallState = d_agent->getFallDetector()->getFallenState();

  if (d_activeScript)
  {
    if (d_activeScript->hasTerminated() == 1.0 && fallState != FallState::STANDUP)
    {
      // We finished playing a script, but still are not standing -- try again
      d_activeScript->reset();
      d_activeScript = nullptr;
    }
  }

  if (d_activeScript == nullptr)
  {
    // First time to run. Determine the direction of the fall and choose the appropriate script to run
    switch (fallState)
    {
      case FallState::BACKWARD: d_activeScript = d_backwardGetUp;    break;
      case FallState::FORWARD:  d_activeScript = d_forwardGetUp;     break;
      case FallState::LEFT:     d_activeScript = d_rollLeftToFront;  break;
      case FallState::RIGHT:    d_activeScript = d_rollRightToFront; break;
      case FallState::STANDUP:
        // Somehow we've been asked to run when already standing.
        return {};
    }
  }

  return {d_activeScript};
}

void GetUpOption::reset()
{
  d_forwardGetUp->reset();
  d_backwardGetUp->reset();
  d_rollLeftToFront->reset();
  d_rollRightToFront->reset();

  d_activeScript = nullptr;
}

double GetUpOption::hasTerminated()
{
  bool standing = d_agent->getFallDetector()->getFallenState() == FallState::STANDUP;

  if (d_activeScript)
    return standing && d_activeScript->hasTerminated() ? 1.0 : 0.0;

  return standing ? 1.0 : 0.0;
}
