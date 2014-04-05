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
  static Setting<double>* d_topAngle = Config::getSetting<double>("options.search-ball.max-height");
  static Setting<double>* d_bottomAngle = Config::getSetting<double>("options.search-ball.min-height");
  static Setting<double>* d_sideAngle = Config::getSetting<double>("options.search-ball.max-side");

  double a = turnSpeed->getValue();
  double maxHeight = d_topAngle->getValue();
  double minHeight = d_bottomAngle->getValue();
  double maxSide = d_sideAngle->getValue();

  double speedX = 10;
  double speedY = 10;

  if (currentPanAngleDegs <= (maxSide - 4))
  {
    // utilize this opportunity to reset ourselves to be looking for the top first
    d_searchTop = true;

    // make head go towards correct side
    d_headModule->moveToDegs(maxSide, currentTiltAngleDegs);
  }
  else
  {
    // if we are too far one way, force the correct movement
    if (currentTiltAngleDegs >= (maxHeight - 2))d_searchTop = false;
    if (currentTiltAngleDegs <= (minHeight + 2))d_searchTop = true;

    // start searching once we have maxed out to one side
    if (d_searchTop)
    {
      if (currentTiltAngleDegs <= (maxHeight - 4))
      {
        // top not reached so aim for it
        //d_headModule->moveToDegs(currentPanAngleDegs, currentTiltAngleDegs + speedY);
        d_headModule->moveToDegs(currentPanAngleDegs, maxHeight);
      }
      else
      {
        // top reached, turn this around next time
        d_searchTop = false;
      }
    }
    else
    {
      if (currentTiltAngleDegs >= (minHeight + 4))
      {
        // bottom not reached so aim for it
        //d_headModule->moveToDegs(currentPanAngleDegs, currentTiltAngleDegs - speedY);
        d_headModule->moveToDegs(currentPanAngleDegs, minHeight);
      }
      else
      {
        // bottom reached, turn this around next time
        d_searchTop = true;
      }
    }

    // finnaly we need to turn ourselves
    d_ambulator->setTurnAngle(a);
  }

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
