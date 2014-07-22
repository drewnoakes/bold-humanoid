#include "circleball.hh"

#include "../../Agent/agent.hh"
#include "../../Drawing/drawing.hh"
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
  d_lookAtFeet(make_shared<LookAtFeet>("look-at-feet", d_headModule)),
  d_lookAtBall(make_shared<LookAtBall>("look-at-ball", agent->getCameraModel(), d_headModule)),
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

  static Setting<double>* rotateUntilAngleDegs = Config::getSetting<double>("options.circle-ball.rotate-until-within-degrees");
  static Setting<double>* maxSpeedX            = Config::getSetting<double>("options.circle-ball.max-speed-x");
  static Setting<double>* maxSpeedY            = Config::getSetting<double>("options.circle-ball.max-speed-y");
  static Setting<double>* turnSpeedA           = Config::getSetting<double>("options.circle-ball.turn-speed-a");
  static Setting<double>* minTranslationSpeedX = Config::getSetting<double>("options.circle-ball.min-translation-speed-x");
  static Setting<double>* minTranslationSpeedY = Config::getSetting<double>("options.circle-ball.min-translation-speed-y");
  static Setting<double>* backUpAngleScale     = Config::getSetting<double>("options.circle-ball.back-up-angle-scale");
  static Setting<double>* backUpMaxDistance    = Config::getSetting<double>("options.circle-ball.back-up-max-distance");

  Vector2d observedBallPos = agentFrame->getBallObservation()->head<2>();
  Vector2d error = d_targetBallPos - observedBallPos;
  Vector2d errorNorm = error.normalized();

  // Scale turn based upon difference between current and target yaw
  auto orientation = State::get<OrientationState>();
  double yawDiffRads = Math::shortestAngleDiffRads(orientation->getYawAngle(), d_targetYaw);
//  cout << "[Circle ball] yaw=" << orientation->getYawAngle() << " target=" << d_targetYaw << " diff=" << yawDiffRads << " posError=" << error.transpose() << endl;
//  a = Math::lerp(fabs(yawDiffRads), 0.0, M_PI/3, 0.0, a);

  Draw::line(Frame::Agent, Vector2d::Zero(), d_targetBallPos, Colour::bgr(0, 128, 255));
  Draw::lineAtAngle(Frame::Agent, d_targetBallPos, M_PI/2 + yawDiffRads, 1.0, Colour::bgr(0, 128, 255));

  bool isLeftTurn = yawDiffRads < 0;

  double x, y, a;

  if (fabs(yawDiffRads) > Math::degToRad(rotateUntilAngleDegs->getValue()))
  {
    // Too much angular error, so rotate

    // Always walk sideways, but not if error becomes too big
    x = (1.0 - Math::clamp(fabs(error.x()), 0.0, 1.0))
      * (isLeftTurn ? -maxSpeedX->getValue() : maxSpeedX->getValue());

    // Try to keep forward distance stable
    y = -Math::lerp(error.y(), 0.0, 0.2, 0.0, maxSpeedY->getValue());
//  y = -Math::clamp(errorNorm.y(), 0.0, 0.4) * maxSpeedYRotation->getValue();

    // Walk backwards from the ball a little when turning
    double backUpFactor = (atan(fabs(backUpAngleScale->getValue() * yawDiffRads)) / (M_PI/2.0));
    ASSERT(backUpFactor >= 0);
    ASSERT(backUpFactor <= 1);
    y -= backUpFactor * backUpMaxDistance->getValue();

    // Turn to keep ball centered
    double errorDir = errorNorm.x() > 0.0 ? 1.0 : -1.0;
    a = errorDir * Math::clamp(fabs(errorNorm.x()), 0.5, 1.0) * turnSpeedA->getValue();
  }
  else
  {
    // Angle is fine, position for kick
    a = 0;
    x = -Math::lerp(error.x() / 0.2, minTranslationSpeedX->getValue(), maxSpeedX->getValue());
    y = -Math::lerp(error.y() / 0.2, minTranslationSpeedY->getValue(), maxSpeedY->getValue());
  }

  x = Math::clamp(x, -maxSpeedX->getValue(), maxSpeedX->getValue());
  y = Math::clamp(y, -maxSpeedY->getValue(), maxSpeedY->getValue());
  a = Math::clamp(a, -turnSpeedA->getValue(), turnSpeedA->getValue());

//  cout << "[Circle ball] " << x << "\t" << y << "\t" << a << endl;

  // NOTE x and y intentionally swapped. 'x' value is also negative as a result of the move
  // direction being inverted.
  d_walkModule->setMoveDir(y, -x);
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
  d_targetBallPos = targetBallPos;

  // Track starting orientation in order to know when we've reached our desired rotation
  auto orientation = State::get<OrientationState>();
  ASSERT(orientation);
  d_targetYaw = Math::normaliseRads(orientation->getYawAngle() + turnAngleRads);

  //cout << "setTurnParams turn=" << turnAngleRads << " targetYaw=" << d_targetYaw << " yaw=" << orientation->getYawAngle() << endl;
}

double CircleBall::hasTerminated()
{
  // First, assert we've turned enough
  double yawErrorRads = State::get<OrientationState>()->getYawAngle() - d_targetYaw;

  if (fabs(yawErrorRads) > Math::degToRad(10)) // TODO magic number!
    return false;

  // If we've turned enough, ensure we are positioned correctly to the ball

  auto agentFrame = State::get<AgentFrameState>();

  // Don't stop yet, first need to find the ball to know whether we are finished
  if (!agentFrame->isBallVisible())
    return false;

  Vector2d posError = d_targetBallPos - agentFrame->getBallObservation()->head<2>();

  //cout << "circleBall posError=" << posError.transpose() << " norm=" << posError.norm() << endl;

  return posError.norm() < 0.02; // TODO magic number
}
