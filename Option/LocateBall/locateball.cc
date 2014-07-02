#include "locateball.hh"

#include "../../Agent/agent.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/CameraFrameState/cameraframestate.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../LookAround/lookaround.hh"
#include "../LookAtBall/lookatball.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

LocateBall::LocateBall(string const& id, Agent* agent)
: Option(id, "LocateBall"),
  d_headModule(agent->getHeadModule()),
  d_lookAtBallOption(make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), d_headModule)),
  d_visibleCount(0),
  d_stepCount(0)
{
  d_lookAroundOption = make_shared<LookAround>("lookAroundForBall", d_headModule, 135, []
  {
    // Slow down considerably when a ball is observed
    return State::get<CameraFrameState>()->isBallVisible() ? 0.3 : 1.0;
  });
}

vector<shared_ptr<Option>> LocateBall::runPolicy(Writer<StringBuffer>& writer)
{
  d_stepCount++;

  auto agentFrame = State::get<AgentFrameState>();

  if (!agentFrame)
    return {};

  if (agentFrame->isBallVisible())
  {
    if (d_visibleCount < 10)
      d_visibleCount++;
  }
  else
  {
    if (d_visibleCount > 0)
      d_visibleCount--;
  }

  // If we've just started this option and have seen a ball, don't look away from it!
  if (d_stepCount < 10 && d_visibleCount != 0)
    return {d_lookAtBallOption};

  // If we are generally seeing a ball, fixate on it
  if (d_visibleCount > 5)
    return {d_lookAtBallOption};

  // Otherwise we are not very confident we're looking at a ball, so keep looking.
  return {d_lookAroundOption};
}

void LocateBall::reset()
{
  d_visibleCount = 0;
  d_stepCount = 0;
  d_lookAroundOption->reset();
  d_lookAtBallOption->reset();
}
