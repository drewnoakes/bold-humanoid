#include "adhocoptiontreebuilder.ih"

/// The robot will stand and kick the ball around in order to learn the
/// outcome of specific kicks given starting ball positions.
shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildKickLearnerFsm(Agent* agent)
{
  // OPTIONS

  auto lookAtFeet = make_shared<LookAtFeet>("look-at-feet", agent->getHeadModule());
  auto lookAtBall = make_shared<LookAtBall>("look-at-ball", agent->getCameraModel(), agent->getHeadModule());
  auto standUp        = make_shared<MotionScriptOption>("stand-up",   agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto kickLeft       = make_shared<MotionScriptOption>("kick-left",  agent->getMotionScriptModule(), "./motionscripts/kick-left.json");
  auto kickRight      = make_shared<MotionScriptOption>("kick-right", agent->getMotionScriptModule(), "./motionscripts/kick-right.json");
  auto kickCrossLeft  = make_shared<MotionScriptOption>("kick-cross-left",  agent->getMotionScriptModule(), "./motionscripts/kick-cross-left.json");
  auto kickCrossRight = make_shared<MotionScriptOption>("kick-cross-right", agent->getMotionScriptModule(), "./motionscripts/kick-cross-right.json");

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "kick-learner");

  auto standUpState = fsm->newState("stand-up", { standUp }, false/*endState*/, true/*startState*/);
  auto waitForBallState = fsm->newState("wait-for-ball", { lookAtFeet });
  auto selectKickState = fsm->newState("select-kick", { });
  auto kickLeftState = fsm->newState("kick-left", { kickLeft, lookAtBall });
  auto kickRightState = fsm->newState("kick-right", { kickRight, lookAtBall });
  auto kickCrossLeftState = fsm->newState("kick-cross-left", { kickCrossLeft, lookAtBall });
  auto kickCrossRightState = fsm->newState("kick-cross-right", { kickCrossRight, lookAtBall });
  auto lookUpForBallState = fsm->newState("look-up", { });
  auto watchBallRollState = fsm->newState("watch-ball-roll", { lookAtBall });
  auto recordOutcomeState = fsm->newState("record-outcome", { });

  // TRANSITIONS

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
    kickUsed = "right-cross";
    return;

//    static auto rng = Math::createUniformRng(0, 4);
//    switch (static_cast<int>(floor(rng())))
//    {
//      case 0: kickUsed = "left"; break;
//      case 1: kickUsed = "right"; break;
//      case 2: kickUsed = "left-cross"; break;
//      case 3: kickUsed = "right-cross"; break;
//    }
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
      log::info("LearnKickResult") << kickUsed
        << ", " << ballStartPos.x() << "," << ballStartPos.y()
        << ", " << ballEndPos.x()   << "," << ballEndPos.y()
        << ", " << (hw ? hw->getCM730State().voltage : -1);
      return true;
    });

  return fsm;
}
