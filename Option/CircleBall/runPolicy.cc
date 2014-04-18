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

  static Setting<double>* maxSpeedX = Config::getSetting<double>("options.circle-ball.max-speed-x");
  static Setting<double>* maxSpeedY = Config::getSetting<double>("options.circle-ball.max-speed-y");
//   static Setting<double>* pGainX = Config::getSetting<double>("options.circle-ball.p-gain-x");
//   static Setting<double>* pGainY = Config::getSetting<double>("options.circle-ball.p-gain-y");
//   static Setting<double>* turnSpeedX = Config::getSetting<double>("options.circle-ball.turn-speed-x");
//   static Setting<double>* turnSpeedY = Config::getSetting<double>("options.circle-ball.turn-speed-y");
  static Setting<double>* turnSpeedA = Config::getSetting<double>("options.circle-ball.turn-speed-a");
//   static Setting<double>* brakeDistance = Config::getSetting<double>("options.circle-ball.brake-distance");

  // TODO set this position based upon which foot is closest to the ball when commencing
  // TODO base the Y position from the idealkicking distance as the position to keep
  auto targetBallPos = Vector2d(0.0, 0.15);
  auto observedBallPos = agentFrame->getBallObservation()->head<2>();
  Vector2d error = targetBallPos - observedBallPos;
  Vector2d errorNorm = error.normalized();


  // Always walk sideways, but not if error becomes too big
  double x = (1.0 - fabs(error.x())) * d_isLeftTurn ? -maxSpeedX->getValue() : maxSpeedX->getValue();

  // Try to keep forward distance stable
  double y = Math::clamp(errorNorm.y(), 0.0, 0.4) * maxSpeedY->getValue();

  // Turn to keep ball centered
  double errorDir = errorNorm.x() > 0.0 ? 1.0 : -1.0;
  double a = errorDir * Math::clamp(fabs(errorNorm.x()), 0.75, 1.0) * turnSpeedA->getValue();

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
  if (!d_isLeftTurn)
    a = Math::clamp(a, 0.0, turnSpeedA->getValue());
  else
    a = Math::clamp(a, -turnSpeedA->getValue(), 0.0);

  // NOTE x and y intentionally swapped. 'x' value is also negative as a result of the move
  // direction being inverted.
  d_walkModule->setMoveDir(-y, -x);
  d_walkModule->setTurnAngle(a);

  writer.String("error").StartArray().Double(error.x()).Double(error.y()).EndArray(2);
  writer.String("moveDir").StartArray().Double(x).Double(y).EndArray(2);
  writer.String("turn").Double(a);

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

  d_walkModule->setMoveDir(x, y);
  d_walkModule->setTurnAngle(a);

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
//   d_walkModule->reset();
}
