#include "adhocoptiontreebuilder.ih"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildPenaltyKeeperFsm(Agent* agent, shared_ptr<OptionTree> tree)
{
  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto leftDive = make_shared<MotionScriptOption>("diveLeft", agent->getMotionScriptModule(), "./motionscripts/dive-left.json");
  auto rightDive = make_shared<MotionScriptOption>("diveRight", agent->getMotionScriptModule(), "./motionscripts/dive-right.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getAmbulator());
  auto lookAroundNarrow = make_shared<LookAround>("lookAroundNarrow", agent->getHeadModule(), 45.0);
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "penalty-keeper"));

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", {stopWalking, lookAroundNarrow});
  auto lookAtBallState = fsm->newState("lookAtBall", {stopWalking, lookAtBall});
  auto leftDiveState = fsm->newState("leftDive", {leftDive});
  auto rightDiveState = fsm->newState("rightDive", {rightDive});

  standUpState
    ->transitionTo(lookForBallState, "standing")
    ->whenTerminated();

  lookForBallState
    ->transitionTo(lookAtBallState, "found")
    ->when(ballFoundConditionFactory);

  lookAtBallState
    ->transitionTo(lookForBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  lookAtBallState
    ->transitionTo(leftDiveState, "ball-left")
    ->when([]()
    {
      return stepUpDownThreshold(3, []()
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && ball->y() < 1.0 && ball->x() < -0.1;
      });
    });

  lookAtBallState
    ->transitionTo(rightDiveState, "ball-right")
    ->when([]()
    {
      return stepUpDownThreshold(3, []()
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && ball->y() < 1.0 && ball->x() > 0.1;
      });
    });

  leftDiveState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  rightDiveState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  ofstream playingOut("fsm-penalty-keeper.dot");
  playingOut << fsm->toDot();

  return fsm;
}
