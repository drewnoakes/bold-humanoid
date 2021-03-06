#include "atball.hh"

#include "../../Agent/agent.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../LookAtFeet/lookatfeet.hh"
#include "../LookAround/lookaround.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

AtBall::AtBall(std::string const& id, Agent* agent)
: Option(id, "AtBall"),
  d_headModule(agent->getHeadModule()),
  d_lookAtFeetOption(make_shared<LookAtFeet>("look-for-ball-at-feet", d_headModule))
{
  d_lookAroundOption = make_shared<LookAround>("look-around-from-ball", d_headModule, 135, [](uint loopCount)
  {
    auto map = State::get<StationaryMapState>();
    auto agentFrame = State::get<AgentFrameState>();

    double speed = 1.0;

    if (map)
    {
      // Don't discount for objects we have seen enough -- otherwise we're running slowly for nothing

      for (auto const& goalPos : agentFrame->getGoalObservations())
      {
        if (map->needMoreSightingsOfGoalPostAt(goalPos.head<2>()))
          speed *= 0.6;
      }

      if (agentFrame->isBallVisible() && map->needMoreSightingsOfBallAt(agentFrame->getBallObservation().value().head<2>()))
        speed *= 0.5;
    }

    double loopScale = LookAround::speedForLoop(loopCount);

    const double minSpeed = 0.1;
    return Math::clamp(loopScale * speed, minSpeed, 1.0);
  });
}

vector<shared_ptr<Option>> AtBall::runPolicy(Writer<StringBuffer>& writer)
{
  // We are standing at the ball.

  // Control the head in order to best populate the stationary map being built.

  auto map = State::get<StationaryMapState>();

  static bool hasMap = true;

  if (!map)
  {
    if (hasMap)
    {
      log::warning("AtBall::runPolicy") << "StationaryMapState unavailable";
      hasMap = false;
    }
    return {};
  }

  hasMap = true;

  static bool busy = false;

  if (!map->hasEnoughGoalPostObservations())
  {
    // We don't see enough goals yet, so continue looking
    busy = true;
    return {d_lookAroundOption};
  }

  if (!map->hasEnoughBallObservations())
  {
    busy = true;
    return {d_lookAtFeetOption};
  }

  if (busy)
  {
    busy = false;
    log::warning("AtBall::runPolicy") << "Policy invoked, yet we have enough goal and ball observations";
  }

  return {};
}

void AtBall::reset()
{
  d_lookAtFeetOption->reset();
  d_lookAroundOption->reset();
}
