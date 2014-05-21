#include "atball.hh"

#include "../../Agent/agent.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/CameraFrameState/cameraframestate.hh"
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
  d_lookAtFeetOption(make_shared<LookAtFeet>("lookForBallAtFeet", d_headModule))
{
  d_lookAroundOption = make_shared<LookAround>("lookAroundFromBall", d_headModule, 135, []()
  {
    auto map = State::get<StationaryMapState>();
    auto cameraFrame = State::get<CameraFrameState>();

    double discountFactor = 0.0;

    if (map && cameraFrame && !map->hasEnoughGoalObservations())
      discountFactor += 0.3 * cameraFrame->getGoalObservationCount();

    if (map && cameraFrame->isBallVisible() && !map->hasEnoughBallObservations())
      discountFactor += 0.6;

    const double minSpeed = 0.1;
    return Math::clamp(1 - discountFactor, minSpeed, 1.0);
  });
}

vector<shared_ptr<Option>> AtBall::runPolicy(Writer<StringBuffer>& writer)
{
  // We are standing at the ball.

  // Control the head in order to best populate the stationary map being built.

  auto map = State::get<StationaryMapState>();

  if (!map)
    return {};

  if (!map->hasEnoughGoalObservations())
  {
    // We don't see enough goals yet, so continue looking
    return {d_lookAroundOption};
  }

  if (!map->hasEnoughBallObservations())
  {
    return {d_lookAtFeetOption};
  }

  log::warning("AtBall::runPolicy") << "Policy invoked, yet we have enough goal and ball observations";

  return {};
}

void AtBall::reset()
{
  d_lookAtFeetOption->reset();
  d_lookAroundOption->reset();
}
