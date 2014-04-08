#include "adhocoptiontreebuilder.ih"

/// The robot will stand and kick the ball around in order to learn the
/// outcome of specific kicks given starting ball positions.
shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildKickLearnerFsm(Agent* agent, shared_ptr<OptionTree> tree)
{
  auto lookAtFeet = make_shared<LookAtFeet>("lookAtFeet", agent->getHeadModule());
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());

  auto standUp       = make_shared<MotionScriptOption>("standUp",   agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto kickLeft      = make_shared<MotionScriptOption>("kickLeft",  agent->getMotionScriptModule(), "./motionscripts/kick-left.json");
  auto kickRight     = make_shared<MotionScriptOption>("kickRight", agent->getMotionScriptModule(), "./motionscripts/kick-right.json");
  auto kickSideLeft  = make_shared<MotionScriptOption>("kickLeft",  agent->getMotionScriptModule(), "./motionscripts/kick-side-left.json");
  auto kickSideRight = make_shared<MotionScriptOption>("kickRight", agent->getMotionScriptModule(), "./motionscripts/kick-side-right.json");

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "kick-learner"));

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto waitForBallState = fsm->newState("wait-for-ball", {lookAtFeet});
  auto selectKickState = fsm->newState("selectKick", {});
  auto kickLeftState = fsm->newState("kickLeft", {kickLeft,lookAtBall});
  auto kickRightState = fsm->newState("kickRight", {kickRight,lookAtBall});
  auto kickSideLeftState  = fsm->newState("kickSideLeft",  {kickSideLeft, lookAtBall});
  auto kickSideRightState = fsm->newState("kickSideRight", {kickSideRight,lookAtBall});
  auto watchBallRollState = fsm->newState("watchBallRoll", {lookAtBall});
  auto recordOutcomeState = fsm->newState("recordOutcome", {});

  constexpr int ObservationCount = 60;
  constexpr double StationaryDeviation = 0.01;

  static Vector3d ballStartPos;
  static Vector3d ballEndPos;
  static string kickUsed;

  standUpState
    ->transitionTo(waitForBallState, "standing")
    ->whenTerminated();

  waitForBallState
    ->transitionTo(selectKickState, "ball-start-observed")
    ->when([]()
    {
      auto avg = make_shared<MovingAverage<Vector3d>>(ObservationCount);
      return [avg]()
      {
        auto agentFrame = State::get<AgentFrameState>();
        if (!agentFrame || !agentFrame->isBallVisible())
          return false;
        ballStartPos = avg->next(*agentFrame->getBallObservation());
        return avg->isMature() && avg->calculateStdDev().norm() < StationaryDeviation;
      };
    });

  selectKickState->transitionTo(kickSideLeftState) ->when([]() { return ballStartPos.x() < 0; });
  selectKickState->transitionTo(kickSideRightState)->when([]() { return ballStartPos.x() > 0; });

  kickLeftState->transitionTo(watchBallRollState)->whenTerminated();
  kickRightState->transitionTo(watchBallRollState)->whenTerminated();
  kickSideLeftState->transitionTo(watchBallRollState)->whenTerminated();
  kickSideRightState->transitionTo(watchBallRollState)->whenTerminated();

  kickLeftState->onEnter.connect([](){ kickUsed = "left"; });
  kickRightState->onEnter.connect([](){ kickUsed = "right"; });
  kickSideLeftState->onEnter.connect([](){ kickUsed = "left-side"; });
  kickSideRightState->onEnter.connect([](){ kickUsed = "right-side"; });

  watchBallRollState
    ->transitionTo(recordOutcomeState, "ball-end-observed")
    ->when([]()
    {
      auto avg = make_shared<MovingAverage<Vector3d>>(ObservationCount);
      return [avg]()
      {
        auto agentFrame = State::get<AgentFrameState>();
        if (!agentFrame || !agentFrame->isBallVisible())
          return false;
        ballEndPos = avg->next(*agentFrame->getBallObservation());
        return avg->isMature() && avg->calculateStdDev().norm() < StationaryDeviation;
      };
    });

  watchBallRollState
    ->transitionTo(waitForBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  recordOutcomeState
    ->transitionTo(waitForBallState)
    ->when([]()
    {
      log::info("LearnKickResult") << kickUsed << " " << ballStartPos.transpose() << ", " << ballEndPos.transpose();
      return true;
    });

  ofstream playingOut("fsm-kick-learner.dot");
  playingOut << fsm->toDot();

  return fsm;
}
