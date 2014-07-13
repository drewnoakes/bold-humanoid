#include "adhocoptiontreebuilder.ih"

#include "../../FieldMap/fieldmap.hh"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildPenaltyKeeperFsm(Agent* agent)
{
  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("stand-up-script", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto leftDive = make_shared<MotionScriptOption>("dive-left", agent->getMotionScriptModule(), "./motionscripts/dive-left.json");
  auto rightDive = make_shared<MotionScriptOption>("dive-right", agent->getMotionScriptModule(), "./motionscripts/dive-right.json");
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto lookAroundNarrow = make_shared<LookAround>("look-around-narrow", agent->getHeadModule(), 80.0, LookAround::speedIfBallVisible(0.05));
  auto lookAtBall = make_shared<LookAtBall>("look-at-ball", agent->getCameraModel(), agent->getHeadModule());

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "penalty-keeper");

  auto standUpState = fsm->newState("stand-up", { standUp }, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("look-for-ball", { stopWalking, lookAroundNarrow });
  auto lookAtBallState = fsm->newState("look-at-ball", { stopWalking, lookAtBall });
  auto leftDiveState = fsm->newState("left-dive", { leftDive });
  auto rightDiveState = fsm->newState("right-dive", { rightDive });

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
