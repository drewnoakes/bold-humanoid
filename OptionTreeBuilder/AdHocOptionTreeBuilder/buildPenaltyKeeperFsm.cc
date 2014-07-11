#include "adhocoptiontreebuilder.ih"

#include "../../FieldMap/fieldmap.hh"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildPenaltyKeeperFsm(Agent* agent)
{
  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto leftDive = make_shared<MotionScriptOption>("diveLeft", agent->getMotionScriptModule(), "./motionscripts/dive-left.json");
  auto rightDive = make_shared<MotionScriptOption>("diveRight", agent->getMotionScriptModule(), "./motionscripts/dive-right.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getWalkModule());
  auto lookAroundNarrow = make_shared<LookAround>("lookAroundNarrow", agent->getHeadModule(), 80.0, [] { return State::get<CameraFrameState>()->isBallVisible() ? 0.05 : 1.0; });
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "penalty-keeper");

  auto standUpState = fsm->newState("standUp", { standUp }, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", { stopWalking, lookAroundNarrow });
  auto lookAtBallState = fsm->newState("lookAtBall", { stopWalking, lookAtBall });
  auto leftDiveState = fsm->newState("leftDive", { leftDive });
  auto rightDiveState = fsm->newState("rightDive", { rightDive });

  // TRANSITIONS

  standUpState
    ->transitionTo(lookForBallState, "standing")
    ->whenTerminated();

  lookForBallState
    ->transitionTo(lookAtBallState, "found")
    ->when(ballFoundConditionFactory);

  lookAtBallState
    ->transitionTo(lookForBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  const static double goalWidth = FieldMap::getGoalY();

  lookAtBallState
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

  lookAtBallState
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
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  rightDiveState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  return fsm;
}
