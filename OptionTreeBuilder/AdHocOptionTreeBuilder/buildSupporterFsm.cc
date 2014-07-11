#include "adhocoptiontreebuilder.ih"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildSupporterFsm(Agent* agent)
{
  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getWalkModule());
  auto lookForBall = make_shared<LookAround>("lookForBall", agent->getHeadModule(), 135.0, [] { return State::get<CameraFrameState>()->isBallVisible() ? 0.15 : 1.0; });
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());
  auto keepPosition = make_shared<KeepPosition>("keepPosition", PlayerRole::Supporter, agent);
  auto searchBall = make_shared<SearchBall>("searchBall", agent->getWalkModule(), agent->getHeadModule());

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "supporter");

  auto standUpState = fsm->newState("standUp", { standUp }, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", { stopWalking, lookForBall });
  auto lookAtBallState = fsm->newState("lookAtBall", { stopWalking, lookAtBall });
  auto circleToFindLostBallState = fsm->newState("lookForBallCircling", { searchBall });
  auto keepPositionState = fsm->newState("keepPosition", { keepPosition });

  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState, lookForBallState, lookForBallState, lookAtBallState });

  // TRANSITIONS

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
    ->transitionTo(keepPositionState, "found")
    ->when([] { return stepUpDownThreshold(10, ballVisibleCondition); });

  keepPositionState
    ->transitionTo(lookForBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  // walk a circle if we don't find the ball within some time limit
  lookForBallState
    ->transitionTo(circleToFindLostBallState, "lost-ball-long")
    ->after(chrono::seconds(8));

  // after 10 seconds of circling, look for the ball again
  circleToFindLostBallState
    ->transitionTo(lookForBallState, "done")
    ->after(chrono::seconds(10));

  // stop turning if the ball comes into view
  circleToFindLostBallState
    ->transitionTo(lookAtBallState, "found")
    ->when([] { return stepUpDownThreshold(5, ballVisibleCondition); });

  return fsm;
}
