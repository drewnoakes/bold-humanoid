#include "support.hh"

#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../Math/math.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

Support::Support(string const& id, shared_ptr<WalkModule> walkModule)
  : Option{id, "Support"},
    d_walkModule{move(walkModule)}
{
  d_yieldDistance = Config::getSetting<double>("options.support.yield-distance");
}

Option::OptionVector Support::runPolicy()
{
  auto agentFrame = State::get<AgentFrameState>();

  if (!agentFrame->isBallVisible())
    return {};

  Vector2d ballPos{agentFrame->getBallObservation()->head<2>()};
  auto ballDir = ballPos.normalized();

  auto yieldPos = ballPos - d_yieldDistance->getValue() * ballDir;
  double walkDist = yieldPos.norm();

  double brakeDist = 0.15;

  double speedScaleDueToDistance = Math::clamp(walkDist / brakeDist, 0.0, 1.0);

  double xSpeed = Math::lerp(speedScaleDueToDistance * yieldPos.y(),
                             9.0,
                             40.0);

  double ySpeed = -Math::lerp(speedScaleDueToDistance * yieldPos.x(),
                             9.0,
                             40.0);

  d_walkModule->setMoveDir(xSpeed, ySpeed);
  d_walkModule->setTurnAngle(0.0);

  return {};
}
