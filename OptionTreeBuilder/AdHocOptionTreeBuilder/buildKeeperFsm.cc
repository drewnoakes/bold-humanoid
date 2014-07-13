#include "adhocoptiontreebuilder.ih"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildKeeperFsm(Agent* agent)
{
  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("stand-up-script", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto locateBall = make_shared<LocateBall>("locate-ball", agent, [] { return State::get<CameraFrameState>()->isBallVisible() ? 0.3 : 0.5; }, 30, 5);
  auto bigStepLeft = make_shared<MotionScriptOption>("big-step-left", agent->getMotionScriptModule(), "./motionscripts/step-left-big.json");
  auto bigStepRight = make_shared<MotionScriptOption>("big-step-right", agent->getMotionScriptModule(), "./motionscripts/step-right-big.json");
  auto leftCrossKick = make_shared<MotionScriptOption>("left-cross-kick-script", agent->getMotionScriptModule(), "./motionscripts/kick-cross-left.json");
  auto rightCrossKick = make_shared<MotionScriptOption>("right-cross-kick-script", agent->getMotionScriptModule(), "./motionscripts/kick-cross-right.json");

  // STATES

  // TODO test this further
  // TODO add logic to kick ball away from goal if close to keeper

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "keeper");

  auto standUpState = fsm->newState("stand-up", { standUp }, false/*endState*/, true/*startState*/);
  auto locateBallState = fsm->newState("locate-ball", { stopWalking, locateBall });
  auto bigStepLeftState = fsm->newState("big-step-left", { bigStepLeft });
  auto bigStepRightState = fsm->newState("big-step-right", { bigStepRight });
  auto leftCrossKickState = fsm->newState("left-cross-kick", { leftCrossKick });
  auto rightCrossKickState = fsm->newState("right-cross-kick", { rightCrossKick });

  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState, locateBallState });
  setPlayerActivityInStates(agent, PlayerActivity::Positioning, { bigStepLeftState, bigStepRightState });

  // TRANSITIONS

  standUpState
    ->transitionTo(locateBallState, "standing")
    ->whenTerminated();

  locateBallState
    ->transitionTo(bigStepLeftState, "ball-left")
    ->when([]
    {
      return trueForMillis(1000, []
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(-0.75, -0.3).contains(ball->x());
      });
    });

  locateBallState
    ->transitionTo(bigStepRightState, "ball-right")
    ->when([]
    {
      return trueForMillis(1000, []
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(0.3, 0.75).contains(ball->x());
      });
    });

  bigStepLeftState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  bigStepRightState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  locateBallState
    ->transitionTo(leftCrossKickState, "clear-left")
    ->when([]
           {
             auto ball = State::get<AgentFrameState>()->getBallObservation();
             return ball && Range<double>(0.0, 0.17).contains(ball->y()) && Range<double>(-0.2, 0).contains(ball->x());
           });

  locateBallState
    ->transitionTo(rightCrossKickState, "clear-right")
    ->when([]
           {
             auto ball = State::get<AgentFrameState>()->getBallObservation();
             return ball && Range<double>(0.0, 0.17).contains(ball->y()) && Range<double>(0, 0.2).contains(ball->x());
           });

  leftCrossKickState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  rightCrossKickState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  return fsm;
}
