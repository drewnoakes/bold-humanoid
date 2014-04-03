#include "circleball.ih"

#include "../LookAtBall/lookatball.hh"
#include "../LookAtFeet/lookatfeet.hh"

using namespace Eigen;

vector<shared_ptr<Option>> CircleBall::runPolicy(Writer<StringBuffer>& writer)
{
  auto agentFrame = State::get<AgentFrameState>();

  if (!agentFrame->isBallVisible())
  {
    // Look at feet (where the ball is likely to be)
    return { d_lookAtFeet };
  }

  // bias towards the foot that will be closest in turn
  // value represents the bias in the foot
  auto targetBallPos = Vector2d(0.1, 0.25);
  auto observedBallPos = agentFrame->getBallObservation()->head<2>();
  Vector2d error = observedBallPos - targetBallPos;

  static Setting<double>* maxSpeedX = Config::getSetting<double>("options.circle-ball.max-speed-x");
  static Setting<double>* maxSpeedY = Config::getSetting<double>("options.circle-ball.max-speed-y");
  static Setting<double>* turnSpeed = Config::getSetting<double>("options.circle-ball.turn-speed");
  static Setting<double>* pGainX = Config::getSetting<double>("options.circle-ball.p-gain-x");
  static Setting<double>* pGainY = Config::getSetting<double>("options.circle-ball.p-gain-y");

  Vector2d control(
    Math::clamp(error.x() * pGainX->getValue(), -maxSpeedX->getValue(), maxSpeedX->getValue()),
    Math::clamp(error.y() * pGainY->getValue(), -maxSpeedY->getValue(), maxSpeedY->getValue())
  );

  double a = d_isLeftTurn ? -turnSpeed->getValue() : turnSpeed->getValue();

  d_ambulator->setMoveDir(Vector2d(control.y(), control.x())); //  NOTE x and y intentionally swapped
  d_ambulator->setTurnAngle(a);

//   writer.String("moveDir").StartArray().Double(x).Double(y).EndArray(2);
//   writer.String("turn").Double(a);

  // Look at ball to make sure we don't lose track of it
  return { d_lookAtBall };

  /*
  auto bodyState = State::get<BodyState>();

  double panAngle = bodyState->getJoint(JointId::HEAD_PAN)->angleRads;
  // TODO don't get this information from the head module, but rather some static model of the body's limits
  double panAngleRange = d_headModule->getLeftLimitRads();
  double panRatio = panAngle / panAngleRange;

  // TODO these move/turn values in config

  double x = 1;
  double y = panRatio < 0 ? 20 : -20;
  double a = panRatio < 0 ? -15 : 15;

  d_ambulator->setMoveDir(Eigen::Vector2d(x, y));
  d_ambulator->setTurnAngle(a);

  writer.String("panAngle").Double(panAngle);
  writer.String("panAngleRange").Double(panAngleRange);
  writer.String("panRatio").Double(panRatio);
  writer.String("moveDir").StartArray().Double(x).Double(y).EndArray(2);
  writer.String("turn").Double(a);

  return {};
  */
}

void CircleBall::setIsLeftTurn(bool leftTurn)
{
  d_isLeftTurn = leftTurn;
}

void CircleBall::reset()
{
  d_ambulator->reset();
}
