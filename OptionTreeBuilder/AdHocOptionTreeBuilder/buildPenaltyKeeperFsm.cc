#include "adhocoptiontreebuilder.hh"

#include "../../FieldMap/fieldmap.hh"
#include "../../Option/LocateBall/locateball.hh"
#include "../../Option/LookAround/lookaround.hh"
#include "../../Option/MotionScriptOption/motionscriptoption.hh"
#include "../../Option/StopWalking/stopwalking.hh"
#include "conditionals.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildPenaltyKeeperFsm(Agent* agent)
{
  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("stand-up-script", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto leftDive = make_shared<MotionScriptOption>("dive-left", agent->getMotionScriptModule(), "./motionscripts/dive-left.json");
  auto rightDive = make_shared<MotionScriptOption>("dive-right", agent->getMotionScriptModule(), "./motionscripts/dive-right.json");
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto locateBall = make_shared<LocateBall>("locate-ball", agent, 80.0, LookAround::speedIfBallVisible(0.15, 0.5), 30, 5);

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "penalty-keeper");

  auto standUpState = fsm->newState("stand-up", { standUp }, false/*endState*/, true/*startState*/);
  auto locateBallState = fsm->newState("locate-ball", { stopWalking, locateBall });
  auto leftDiveState = fsm->newState("left-dive", { leftDive });
  auto rightDiveState = fsm->newState("right-dive", { rightDive });

  // TRANSITIONS

  standUpState
    ->transitionTo(locateBallState, "standing")
    ->whenTerminated();

  const static double goalWidth = FieldMap::getGoalY();

  locateBallState
    ->transitionTo(leftDiveState, "ball-left")
    ->when([]
    {
      return stepUpDownThreshold(3, []
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball &&
               ball->y() < 1.0 && ball->y() > -0.2 &&
               ball->x() < -0.1 && ball->x() > -goalWidth/1.5;
      });
    });

  locateBallState
    ->transitionTo(rightDiveState, "ball-right")
    ->when([]
    {
      return stepUpDownThreshold(3, []
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball &&
               ball->y() < 1.0 && ball->y() > -0.2 &&
              ball->x() > 0.1 && ball->x() < goalWidth/1.5;
      });
    });

  leftDiveState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  rightDiveState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  return fsm;
}
