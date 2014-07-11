#include "awaittheirkickoff.hh"

#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/GameState/gamestate.hh"

using namespace bold;
using namespace std;
using namespace rapidjson;
using namespace Eigen;

AwaitTheirKickOff::AwaitTheirKickOff(string id)
: Option(id, "AwaitTheirKickOff"),
  d_ballPosition((ushort)Config::getStaticValue<int>("options.await-their-kick-off.ball-pos-window-size")),
  d_ballInitialPosition(),
  d_requiredMoveDistance(Config::getSetting<double>("options.await-their-kick-off.required-ball-movement"))
{}

vector<shared_ptr<Option>> AwaitTheirKickOff::runPolicy(Writer<StringBuffer>& writer)
{
  // Observe the ball position
  auto agentFrame = State::get<AgentFrameState>();

  ASSERT(agentFrame);

  if (agentFrame->isBallVisible())
  {
    // Integrate the observed position
    d_ballPosition.next(agentFrame->getBallObservation()->head<2>());

    if (!d_ballInitialPosition.hasValue())
    {
      // No initial ball position estimate yet.
      if (d_ballPosition.isMature())
      {
        // We have enough observations to make our estimate
        d_ballInitialPosition = d_ballPosition.getAverage();

        log::info("AwaitTheirKickOff::runPolicy") << "Initial ball position estimate made: " << d_ballInitialPosition->x() << ", " << d_ballInitialPosition->y();

        // Reset our average and start building another which we can use to transition out of waiting when
        // we detect the ball has moved.
        d_ballPosition.reset();
      }
    }
  }

  return {};
}

double AwaitTheirKickOff::hasTerminated()
{
  auto game = State::get<GameState>();

  // We should have used GameState to get into this state in the first place!
  ASSERT(game);

  // If the time period has elapsed, then terminate this state
  if (!game->isWithinTenSecondsOfKickOff(Team::Them))
  {
    log::info("AwaitTheirKickOff::hasTerminated") << "Ten second period has elapsed";
    return 1.0;
  }

  // Has the ball moved far enough to abort the 10 second wait?
  if (d_ballInitialPosition.hasValue() && d_ballPosition.isMature())
  {
    // We have an initial position estimate, and now have a mature estimate of the current ball's position
    auto distance = (*d_ballInitialPosition - d_ballPosition.getAverage()).norm();

    if (distance > d_requiredMoveDistance->getValue())
    {
      log::info("AwaitTheirKickOff::hasTerminated") << "Ball has moved " << distance << " from it's initial position to: " << d_ballPosition.getAverage().x() << ", " << d_ballPosition.getAverage().y();
      return 1.0;
    }
  }

  return 0.0;
}

void AwaitTheirKickOff::reset()
{
  d_ballPosition.reset();
  d_ballInitialPosition.reset();
}
