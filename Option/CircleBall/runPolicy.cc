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
  static Setting<double>* pGainX = Config::getSetting<double>("options.circle-ball.p-gain-x");
  static Setting<double>* pGainY = Config::getSetting<double>("options.circle-ball.p-gain-y");
  static Setting<double>* turnSpeedX = Config::getSetting<double>("options.circle-ball.turn-speed-x");
  static Setting<double>* turnSpeedY = Config::getSetting<double>("options.circle-ball.turn-speed-y");
  static Setting<double>* turnSpeedA = Config::getSetting<double>("options.circle-ball.turn-speed-a");
  static Setting<double>* brakeDistance = Config::getSetting<double>("options.circle-ball.brake-distance");

  // TODO set this position based upon which foot is closest to the ball when commencing
  // TODO base the Y position from the idealkicking distance as the position to keep
  auto targetBallPos = Vector2d(0.1, 0.20);
  auto observedBallPos = agentFrame->getBallObservation()->head<2>();
  Vector2d error = observedBallPos - targetBallPos;

  // Alpha controls how much turning is attempted. Max turn occurs when we have
  // no positional error for the ball.
  // If the error is greater than the brake distance, then turning is disabled
  // to allow the position to be corrected.
  // Value is linearly interpolated as a ratio of error length to brake distance.
  double alpha = Math::clamp(1.0 - (error.norm()/brakeDistance->getValue()), 0.0, 1.0);

  // Set movement speed in x/y based on error distance
  double x = error.x() * pGainX->getValue();
  double y = error.y() * pGainY->getValue();

  // Add turn movement, based upon ratio of error
  x += alpha * turnSpeedX->getValue();
  y += alpha * turnSpeedY->getValue();

  // Blend turn speed
  double a = alpha * (d_isLeftTurn ? -turnSpeedA->getValue() : turnSpeedA->getValue());

  // Clamp movement direction to within maximums
  x = Math::clamp(x, -maxSpeedX->getValue(), maxSpeedX->getValue());
  y = Math::clamp(y, -maxSpeedY->getValue(), maxSpeedY->getValue());

  cout << "ERR " << error.x() << ", " << error.y()
       << " CTL " << x << ", " << y
       << " ALPHA " << alpha
       << endl;

  // NOTE x and y intentionally swapped. 'x' value is also negative as a result of the move
  // direction being inverted.
  d_ambulator->setMoveDir(Vector2d(-y, x));
  d_ambulator->setTurnAngle(a);

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
