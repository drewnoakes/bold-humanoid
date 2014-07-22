#include "adhocoptiontreebuilder.hh"

#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../Option/ApproachBall/approachball.hh"
#include "../../Option/CircleBall/circleball.hh"
#include "../../Option/LocateBall/locateball.hh"
#include "../../Option/LookAround/lookaround.hh"
#include "../../Option/StopWalking/stopwalking.hh"
#include "../../Option/MotionScriptOption/motionscriptoption.hh"
#include "../../State/state.hh"
#include "conditionals.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

/// The robot will approach the ball then circle it
shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildBallCirclerFsm(Agent* agent)
{
  // OPTIONS

  auto standUp      = make_shared<MotionScriptOption>("stand-up", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto locateBall   = make_shared<LocateBall>("locate-ball", agent, 135.0, LookAround::speedIfBallVisible(0.15));
  auto lookAtBall   = make_shared<LookAtBall>("look-at-ball", agent->getCameraModel(), agent->getHeadModule());
  auto approachBall = make_shared<ApproachBall>("approach-ball", agent->getWalkModule(), agent->getBehaviourControl());
  auto circleBall   = make_shared<CircleBall>("circle-ball",  agent);
  auto stopWalking  = make_shared<StopWalking>("stop-walking", agent->getWalkModule());

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "ball-circler");

  auto standUpState = fsm->newState("stand-up", { standUp }, false/*endState*/, true/*startState*/);
  auto locateBallState = fsm->newState("locate-ball", { locateBall });
  auto approachBallState = fsm->newState("approach-ball", { approachBall, lookAtBall });
  auto atBallState = fsm->newState("at-ball", { stopWalking });
  auto circleBallState = fsm->newState("circle-ball", { circleBall });
  auto doneState = fsm->newState("done", { stopWalking });

  // TRANSITIONS

  standUpState
    ->transitionTo(locateBallState, "standing")
    ->whenTerminated();

  // start approaching the ball when we have the confidence that it's really there
  locateBallState
    ->transitionTo(approachBallState, "found-ball")
    ->when([] { return stepUpDownThreshold(10, ballVisibleCondition); });

  // stop walking to ball once we're close enough
  approachBallState
    ->transitionTo(atBallState, "near-ball")
    ->when(ballIsStoppingDistance);

  approachBallState
    ->transitionTo(locateBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  atBallState
    ->transitionTo(circleBallState, "start-circling")
    ->when([approachBallState,circleBall]
      {
        if (approachBallState->timeSinceStart() >= std::chrono::seconds(3))
        {
          static auto rng = Math::createUniformRng(0, M_PI, true);
          double rads = rng();
          Vector2d pos = rng() > (M_PI/2.0) ? Vector2d(-0.065, 0.106) : Vector2d(0.065, 0.106);
          log::info("ballCircler") << "Turning " << Math::radToDeg(rads) << " degrees with ball position " << pos.x() << "," << pos.y();
          circleBall->setTurnParams(rads, pos);
          return true;
        }
        return false;
      });

  circleBallState
    ->transitionTo(doneState, "done")
    ->whenTerminated();

  doneState
    ->transitionTo(standUpState)
    ->when(ballNotVisibleCondition);

  return fsm;
}
