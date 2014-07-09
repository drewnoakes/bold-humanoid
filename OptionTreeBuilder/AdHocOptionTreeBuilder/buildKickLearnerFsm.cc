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
  auto kickCrossLeft = make_shared<MotionScriptOption>("kickCrossLeft",  agent->getMotionScriptModule(), "./motionscripts/kick-cross-left.json");
  auto kickCrossRight = make_shared<MotionScriptOption>("kickCrossRight", agent->getMotionScriptModule(), "./motionscripts/kick-cross-right.json");

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "kick-learner"));

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto waitForBallState = fsm->newState("wait-for-ball", {lookAtFeet});
  auto selectKickState = fsm->newState("selectKick", {});
  auto kickLeftState = fsm->newState("kickLeft", {kickLeft,lookAtBall});
  auto kickRightState = fsm->newState("kickRight", {kickRight,lookAtBall});
  auto kickCrossLeftState = fsm->newState("kickCrossLeft", {kickCrossLeft, lookAtBall});
  auto kickCrossRightState = fsm->newState("kickCrossRight", {kickCrossRight,lookAtBall});
  auto lookUpForBallState = fsm->newState("lookUp", {});
  auto watchBallRollState = fsm->newState("watchBallRoll", {lookAtBall});
  auto recordOutcomeState = fsm->newState("recordOutcome", {});

  constexpr int observationCount = 60;
  constexpr double stationaryDeviation = 0.01;

  static Vector3d ballStartPos;
  static Vector3d ballEndPos;
  static string kickUsed;

  standUpState
    ->transitionTo(waitForBallState, "standing")
    ->whenTerminated();

  waitForBallState
    ->transitionTo(selectKickState, "ball-start-observed")
    ->when([observationCount]
    {
      auto avg = make_shared<MovingAverage<Vector3d>>(observationCount);
      return [avg]
      {
        auto agentFrame = State::get<AgentFrameState>();
        if (!agentFrame || !agentFrame->isBallVisible())
          return false;
        ballStartPos = avg->next(*agentFrame->getBallObservation());
        return avg->isMature() && ballStartPos.y() < 0.22 && avg->calculateStdDev().norm() < stationaryDeviation;
      };
    });

  selectKickState->onEnter.connect([]
  {
    // HACK only do right-kick for now
    kickUsed = "right";
    return;

    static auto rng = Math::createUniformRng(0, 4);
    switch (static_cast<int>(floor(rng())))
    {
      case 0: kickUsed = "left"; break;
      case 1: kickUsed = "right"; break;
      case 2: kickUsed = "left-cross"; break;
      case 3: kickUsed = "right-cross"; break;
    }
  });

  selectKickState->transitionTo(kickLeftState) ->when([] { return kickUsed == "left"; });
  selectKickState->transitionTo(kickRightState)->when([] { return kickUsed == "right"; });
  selectKickState->transitionTo(kickCrossLeftState) ->when([] { return kickUsed == "left-cross"; });
  selectKickState->transitionTo(kickCrossRightState)->when([] { return kickUsed == "right-cross"; });

  kickLeftState->transitionTo(lookUpForBallState)->whenTerminated();
  kickRightState->transitionTo(lookUpForBallState)->whenTerminated();
  kickCrossLeftState->transitionTo(lookUpForBallState)->whenTerminated();
  kickCrossRightState->transitionTo(lookUpForBallState)->whenTerminated();

  lookUpForBallState->onEnter.connect([agent]
  {
    if (kickUsed=="left")
      agent->getHeadModule()->moveToDegs(5, 20);
    else if (kickUsed=="right")
      agent->getHeadModule()->moveToDegs(-5, 20);
    else if (kickUsed=="left-cross")
      agent->getHeadModule()->moveToDegs(-35, 35);
    else if (kickUsed=="right-cross")
      agent->getHeadModule()->moveToDegs(35, 35);
  });

  lookUpForBallState
    ->transitionTo(watchBallRollState)
    ->after(chrono::milliseconds(200));

  watchBallRollState
    ->transitionTo(recordOutcomeState, "ball-end-observed")
    ->when([observationCount]
    {
      auto avg = make_shared<MovingAverage<Vector3d>>(observationCount);
      return [avg]
      {
        auto agentFrame = State::get<AgentFrameState>();
        if (!agentFrame || !agentFrame->isBallVisible())
          return false;
        ballEndPos = avg->next(*agentFrame->getBallObservation());
        return avg->isMature() && avg->calculateStdDev().norm() < stationaryDeviation;
      };
    });

  watchBallRollState
    ->transitionTo(waitForBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  recordOutcomeState
    ->transitionTo(waitForBallState)
    ->when([]
    {
      auto hw = State::get<HardwareState>();
      log::info("LearnKickResult") << kickUsed << " " << ballStartPos.transpose() << ", " << ballEndPos.transpose() << ", " << (hw ? hw->getCM730State().voltage : -1);
      return true;
    });

  ofstream playingOut("fsm-kick-learner.dot");
  playingOut << fsm->toDot();

  return fsm;
}
