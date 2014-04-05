#include "searchball.ih"

#include "../LookAtBall/lookatball.hh"
#include "../LookAtFeet/lookatfeet.hh"
#include "../../StateObject/BodyState/bodystate.hh"

using namespace Eigen;

vector<shared_ptr<Option>> SearchBall::runPolicy(Writer<StringBuffer>& writer)
{
  auto agentFrame = State::get<AgentFrameState>();

  auto body = State::get<BodyState>();
  double currentPanAngleDegs = Math::radToDeg(body->getJoint(JointId::HEAD_PAN)->angleRads);
  double currentTiltAngleDegs = Math::radToDeg(body->getJoint(JointId::HEAD_TILT)->angleRads);

  static Setting<double>* turnSpeed = Config::getSetting<double>("options.search-ball.turn-speed");
  static Setting<double>* d_topAngle = Config::getSetting<double>("options.search-ball.max-target-height");
  static Setting<double>* d_bottomAngle = Config::getSetting<double>("options.search-ball.min-target-height");
  static Setting<double>* d_sideAngle = Config::getSetting<double>("options.search-ball.max-target-side");
  static Setting<double>* d_speedX = Config::getSetting<double>("options.search-ball.speed-x");
  static Setting<double>* d_speedY = Config::getSetting<double>("options.search-ball.speed-y");

  double a = turnSpeed->getValue();
  double maxTargetHeight = d_topAngle->getValue();
  double minTargetHeight = d_bottomAngle->getValue();
  double maxTargetSide = d_sideAngle->getValue();
  double speedX = d_speedX->getValue();
  double speedY = d_speedY->getValue();

  // make sure head isfully turned in the direction we are turning to maximize our chances of seeing the ball
  if (d_isLeftTurn ? currentPanAngleDegs <= (maxTargetSide - speedX) : currentPanAngleDegs >= (-maxTargetSide + speedX))
  {
    // utilize this opportunity to reset ourselves to be looking for the top first
    d_searchTop = true;

    // make head go towards correct side, overshoot
    d_headModule->moveToDegs(d_isLeftTurn ? maxTargetSide + speedX : -maxTargetSide - speedX, currentTiltAngleDegs);
  }
  else
  {
    // if we are too far one way, force the need to correct movement
    if (currentTiltAngleDegs >= (maxTargetHeight - 2))d_searchTop = false;
    if (currentTiltAngleDegs <= (minTargetHeight + 2))d_searchTop = true;

    // start searching once we have maxed out to one side
    if (d_searchTop)
    {
      if (currentTiltAngleDegs <= (maxTargetHeight - speedY))
      {
        // top not reached so aim for it, overshoot
        d_headModule->moveToDegs(currentPanAngleDegs, maxTargetHeight + speedY);
      }
      else
      {
        // top reached, turn this around next time
        d_searchTop = false;
      }
    }
    else
    {
      if (currentTiltAngleDegs >= (minTargetHeight + speedY))
      {
        // bottom not reached so aim for it, overshoot
        d_headModule->moveToDegs(currentPanAngleDegs, minTargetHeight - speedY);
      }
      else
      {
        // bottom reached, turn this around next time
        d_searchTop = true;
      }
    }

    // finnaly we need to turn ourselves
    d_ambulator->setTurnAngle(d_isLeftTurn ? a : -a);
  }

  // return nothing
  return {};
}

void SearchBall::setIsLeftTurn(bool leftTurn)
{
  d_isLeftTurn = leftTurn;
}

void SearchBall::reset()
{
  d_ambulator->reset();
}
