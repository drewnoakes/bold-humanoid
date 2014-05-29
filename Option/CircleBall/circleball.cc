#include "circleball.hh"

#include "../../Agent/agent.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/OrientationState/orientationstate.hh"
#include "../LookAtBall/lookatball.hh"
#include "../LookAtFeet/lookatfeet.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

CircleBall::CircleBall(std::string const& id, Agent* agent)
: Option(id, "CircleBall"),
  d_walkModule(agent->getWalkModule()),
  d_headModule(agent->getHeadModule()),
  d_lookAtFeet(make_shared<LookAtFeet>("lookAtFeet", d_headModule)),
  d_lookAtBall(make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), d_headModule)),
  d_turnAngleRads(0),
  d_targetBallPos(0.0, 0.15),
  d_targetYaw(0)
{}

vector<shared_ptr<Option>> CircleBall::runPolicy(Writer<StringBuffer>& writer)
{
  auto agentFrame = State::get<AgentFrameState>();

  ASSERT(agentFrame);

  if (!agentFrame->isBallVisible())
  {
    // Look at feet (where the ball is likely to be)
    return { d_lookAtFeet };
  }

  static Setting<double>* maxSpeedX = Config::getSetting<double>("options.circle-ball.max-speed-x");
  static Setting<double>* maxSpeedY = Config::getSetting<double>("options.circle-ball.max-speed-y");
  static Setting<double>* turnSpeedA = Config::getSetting<double>("options.circle-ball.turn-speed-a");

  auto observedBallPos = agentFrame->getBallObservation()->head<2>();
  Vector2d error = d_targetBallPos - observedBallPos;
  Vector2d errorNorm = error.normalized();

  bool isLeftTurn = d_turnAngleRads < 0;

  // Always walk sideways, but not if error becomes too big
  double x = (1.0 - fabs(error.x())) * isLeftTurn ? -maxSpeedX->getValue() : maxSpeedX->getValue();

  // Try to keep forward distance stable
  double y = Math::clamp(errorNorm.y(), 0.0, 0.4) * maxSpeedY->getValue();

  // Turn to keep ball centered
  double errorDir = errorNorm.x() > 0.0 ? 1.0 : -1.0;
  double a = errorDir * Math::clamp(fabs(errorNorm.x()), 0.75, 1.0) * turnSpeedA->getValue();

  // Scale turn based upon difference between current and target yaw
  auto orientation = State::get<OrientationState>();
  double yawDiffRads = Math::shortestAngleDiffRads(orientation->getYawAngle(), d_targetYaw);
  a = Math::lerp(yawDiffRads, 0.0, M_PI/3, 0.0, a);

  /*
  // Keep distance same
  // Alpha controls how much turning is attempted. Max turn occurs when we have
  // no positional error for the ball.
  // If the error is greater than the brake distance, then turning is disabled
  // to allow the position to be corrected.
  // Value is linearly interpolated as a ratio of error length to brake distance.
  double alpha = Math::clamp(1.0 - (error.norm()/brakeDistance->getValue()), 0.5, 1.0);

  // Set movement speed in x/y based on error distance
  //double x = error.x() * pGainX->getValue();
  double x = pGainX->getValue() - errorNorm.x() * pGainX->getValue();;
  double y = error.y() * pGainY->getValue();

  // Add turn movement, based upon ratio of error
  //x += alpha * turnSpeedX->getValue();
  //y += alpha * turnSpeedY->getValue();

  // Blend turn speed
  double a = (1.0 - fabs(errorNorm.x())) * (d_isLeftTurn ? -turnSpeedA->getValue() : turnSpeedA->getValue());

  // Clamp movement direction to within maximums

  x = d_isLeftTurn ? -x : x;
  */

  x = Math::clamp(x, -maxSpeedX->getValue(), maxSpeedX->getValue());
  y = Math::clamp(y, -maxSpeedY->getValue(), maxSpeedY->getValue());
  a = Math::clamp(a, -turnSpeedA->getValue(), turnSpeedA->getValue());

  // NOTE x and y intentionally swapped. 'x' value is also negative as a result of the move
  // direction being inverted.
  d_walkModule->setMoveDir(-y, -x);
  d_walkModule->setTurnAngle(a);

  writer.String("yawError").Double(yawDiffRads);
  writer.String("posError").StartArray().Double(error.x()).Double(error.y()).EndArray(2);
  writer.String("moveDir").StartArray().Double(x).Double(y).EndArray(2);
  writer.String("turn").Double(a);

  // Look at ball to make sure we don't lose track of it
  return { d_lookAtBall };
}

void CircleBall::setTurnParams(double turnAngleRads, Eigen::Vector2d targetBallPos)
{
  d_turnAngleRads = turnAngleRads;
  d_targetBallPos = targetBallPos;

  // Track starting orientation in order to know when we've reached our desired rotation
  auto orientation = State::get<OrientationState>();
  ASSERT(orientation);
  d_targetYaw = Math::normaliseRads(orientation->getYawAngle() + turnAngleRads);
}

double CircleBall::hasTerminated()
{
  // First, assert we've turned enough
  double yawErrorRads = State::get<OrientationState>()->getYawAngle() - d_targetYaw;

  if (fabs(yawErrorRads) > Math::degToRad(10)) // TODO magic number!
    return false;

  // If we've turned enough, ensure we are positioned correctly to the ball

  auto agentFrame = State::get<AgentFrameState>();

  // If we cannot see the ball, there's no point continuing
  if (!agentFrame->isBallVisible())
    return true;

  Vector2d posError = d_targetBallPos - agentFrame->getBallObservation()->head<2>();

  return posError.norm() < 0.02; // TODO magic number
}
