#include "locateball.hh"

#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../LookAround/lookaround.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

LocateBall::LocateBall(string const& id, Agent* agent, double sideAngle, function<double(uint)> speedCallback, uint maxCount, uint thresholdCount)
: Option(id, "LocateBall"),
  d_headModule(agent->getHeadModule()),
  d_lookAroundOption(make_shared<LookAround>("look-around-for-ball", d_headModule, sideAngle, speedCallback)),
  d_lookAtBallOption(make_shared<LookAtBall>("look-at-ball", agent->getCameraModel(), d_headModule)),
  d_visibleCount(0),
  d_stepCount(0),
  d_maxCount(maxCount),
  d_thresholdCount(thresholdCount)
{}

vector<shared_ptr<Option>> LocateBall::runPolicy(Writer<StringBuffer>& writer)
{
  d_stepCount++;

  auto agentFrame = State::get<AgentFrameState>();

  if (!agentFrame)
    return {};

  if (agentFrame->isBallVisible())
  {
    if (d_visibleCount < d_maxCount)
      d_visibleCount++;
  }
  else
  {
    if (d_visibleCount > 0)
      d_visibleCount--;
  }

  // If we've just started this option and have seen a ball, don't look away from it!
  if (d_stepCount < d_maxCount && d_visibleCount != 0)
    return {d_lookAtBallOption};

  // If we are generally seeing a ball, fixate on it
  if (d_visibleCount > d_thresholdCount)
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
