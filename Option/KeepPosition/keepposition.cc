#include "keepposition.hh"

#include "../../Ambulator/ambulator.hh"
#include "../../Config/config.hh"
#include "../../Option/ApproachBall/approachball.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"

using namespace bold;
using namespace std;
using namespace rapidjson;
using namespace Eigen;

KeepPosition::KeepPosition(string id, PlayerRole role, shared_ptr<Ambulator> ambulator)
: Option(id, "KeepPosition"),
  d_ambulator(ambulator),
  d_role(role),
  d_approachBall(make_shared<ApproachBall>("approachBallKeepingPosition", ambulator)),
  d_supporterSpacing(Config::getSetting<double>("options.keep-position.spacing"))
{}

vector<shared_ptr<Option>> KeepPosition::runPolicy(Writer<StringBuffer>& writer)
{
  // TODO require certain delta if not walking in order to start walking -- don't try and correct by 3cm for example

  auto team = State::get<TeamState>();

  if (!team || team->empty())
  {
    log::warning("KeepPosition::runPolicy") << "Should not be keeping position when no team mates visible";
    return {};
  }

  auto agentFrame = State::get<AgentFrameState>();
  if (!agentFrame->isBallVisible())
  {
    d_ambulator->setMoveDir(Vector2d(0, 0));
    return {};
  }

  //
  // Determine our rank among teammates
  //

  auto observers = team->getBallObservers();

  sort(observers.begin(), observers.end(),
    [](PlayerState const& a, PlayerState const& b) -> bool
    {
      return a.ballRelative->norm() > b.ballRelative->norm();
    });

  int rank = 1;
  double dist = agentFrame->getBallObservation()->norm();

  for (PlayerState const& player : observers)
  {
    if (player.ballRelative->norm() < dist)
      rank++;
  }

  // Calculate max distance from ball as multiple of supporter spacing distance
  auto maxDistance = d_supporterSpacing->getValue() * rank;

  // Ensure we are at at least the max distance from the ball

  if (dist < maxDistance)
  {
    d_ambulator->setMoveDir(Vector2d(0, 0));
    return {};
  }

  d_approachBall->setStopDistance(maxDistance);

  return {d_approachBall};
}
