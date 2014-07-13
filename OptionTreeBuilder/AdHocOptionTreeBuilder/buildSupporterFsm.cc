#include "adhocoptiontreebuilder.ih"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildSupporterFsm(Agent* agent)
{
  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("stand-up-script", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto lookForBall = make_shared<LookAround>("look-for-ball", agent->getHeadModule(), 135.0, LookAround::speedIfBallVisible(0.15));
  auto lookAtBall = make_shared<LookAtBall>("look-at-ball", agent->getCameraModel(), agent->getHeadModule());
  auto keepPosition = make_shared<KeepPosition>("keep-position", PlayerRole::Supporter, agent);
  auto searchBall = make_shared<SearchBall>("search-ball", agent->getWalkModule(), agent->getHeadModule());

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "supporter");

  auto standUpState = fsm->newState("stand-up", { standUp }, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("look-for-ball", { stopWalking, lookForBall });
  auto lookAtBallState = fsm->newState("look-at-ball", { stopWalking, lookAtBall });
  auto circleToFindLostBallState = fsm->newState("look-for-ball-circling", { searchBall });
  auto keepPositionState = fsm->newState("keep-position", { keepPosition });

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
