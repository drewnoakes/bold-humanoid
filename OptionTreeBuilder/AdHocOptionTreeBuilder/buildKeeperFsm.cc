#include "adhocoptiontreebuilder.ih"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildKeeperFsm(Agent* agent)
{
  // KICKS

  vector<shared_ptr<Kick const>> kicks = { Kick::getById("cross-right"), Kick::getById("cross-left") };

  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("stand-up-script", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto locateBall = make_shared<LocateBall>("locate-ball", agent, [] { return State::get<CameraFrameState>()->isBallVisible() ? 0.3 : 0.5; }, 30, 5);
  auto bigStepLeft = make_shared<MotionScriptOption>("big-step-left", agent->getMotionScriptModule(), "./motionscripts/step-left-big.json");
  auto bigStepRight = make_shared<MotionScriptOption>("big-step-right", agent->getMotionScriptModule(), "./motionscripts/step-right-big.json");
  auto clearGoalKick = make_shared<MotionScriptOption>("clear-goal-kick", agent->getMotionScriptModule());

  // STATES

  // TODO test this further
  // TODO add logic to kick ball away from goal if close to keeper

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "keeper");

  auto standUpState = fsm->newState("stand-up", { standUp }, false/*endState*/, true/*startState*/);
  auto locateBallState = fsm->newState("locate-ball", { stopWalking, locateBall });
  auto bigStepLeftState = fsm->newState("big-step-left", { bigStepLeft });
  auto bigStepRightState = fsm->newState("big-step-right", { bigStepRight });
  auto clearGoalKickState = fsm->newState("clear-goal-kick", { clearGoalKick });

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
    ->transitionTo(clearGoalKickState, "can-kick")
    ->when([clearGoalKick,kicks]
    {
      auto ball = State::get<AgentFrameState>()->getBallObservation();

      if (!ball)
        return false;

      // Take the first possible kick
      // TODO should we take the one that travels the furthest?
      for (auto const& kick : kicks)
      {
        if (kick->estimateEndPos(ball->head<2>()).hasValue())
        {
          // We can perform this kick
          clearGoalKick->setMotionScript(kick->getMotionScript());
          return true;
        }
      }
      return false;
    });

  clearGoalKickState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  return fsm;
}
