#include "adhocoptiontreebuilder.ih"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildKeeperFsm(Agent* agent, shared_ptr<OptionTree> tree)
{
  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getAmbulator());
  auto approachBall = make_shared<ApproachBall>("approachBall", agent->getAmbulator());
  auto circleBall = make_shared<CircleBall>("circleBall", agent->getAmbulator(), agent->getHeadModule());
  auto lookAroundNarrow = make_shared<LookAround>("lookAroundNarrow", agent->getHeadModule(), 45.0);
  auto lookForGoal = make_shared<LookAround>("lookForGoal", agent->getHeadModule(), 100.0, []() { return 1 - 0.33*State::get<CameraFrameState>()->getGoalObservationCount(); });
  auto lookForBall = make_shared<LookAround>("lookForBall", agent->getHeadModule(), 135.0, []() { return State::get<CameraFrameState>()->isBallVisible() ? 0.25 : 1.0; });
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());
  auto lookAtFeet = make_shared<LookAtFeet>("lookAtFeet", agent->getHeadModule());
  auto lookAtGoal = make_shared<LookAtGoal>("lookAtGoal", agent->getCameraModel(), agent->getHeadModule());

  auto leftDive = make_shared<MotionScriptOption>("diveLeft", agent->getMotionScriptModule(), "./motionscripts/dive-left.json");
  auto rightDive = make_shared<MotionScriptOption>("diveRight", agent->getMotionScriptModule(), "./motionscripts/dive-right.json");
  auto bigStepLeft = make_shared<MotionScriptOption>("bigStepLeft", agent->getMotionScriptModule(), "./motionscripts/step-left-big.json");
  auto bigStepRight = make_shared<MotionScriptOption>("bigStepRight", agent->getMotionScriptModule(), "./motionscripts/step-right-big.json");

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "keeper"));

  // TODO test this further
  // TODO add logic to kick ball away from goal if close to keeper

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", {stopWalking, lookForBall});
  auto lookAtBallState = fsm->newState("lookAtBall", {stopWalking, lookAtBall});
  auto bigStepLeftState = fsm->newState("bigStepLeft", {bigStepLeft});
  auto bigStepRightState = fsm->newState("bigStepRight", {bigStepRight});

  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState, lookForBallState, lookForBallState, lookAtBallState, bigStepLeftState, bigStepRightState });

  standUpState
    ->transitionTo(lookForBallState, "standing")
    ->whenTerminated();

  lookForBallState
    ->transitionTo(lookAtBallState, "found")
    ->when(ballFoundConditionFactory);

  lookAtBallState
    ->transitionTo(lookForBallState, "lost")
    ->when(ballLostConditionFactory);

  lookAtBallState
    ->transitionTo(bigStepLeftState, "ball-left")
    ->when([]()
    {
      return trueForMillis(1000, []()
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(-0.75, -0.3).contains(ball->x());
      });
    });

  lookAtBallState
    ->transitionTo(bigStepRightState, "ball-right")
    ->when([]()
    {
      return trueForMillis(1000, []()
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(0.3, 0.75).contains(ball->x());
      });
    });

  bigStepLeftState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  bigStepRightState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  ofstream playingOut("fsm-keeper.dot");
  playingOut << fsm->toDot();

  return fsm;
}
