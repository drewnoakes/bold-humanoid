#include "adhocoptiontreebuilder.ih"

shared_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(Agent* agent)
{
  unsigned uniformNumber   = agent->getUniformNumber();
  unsigned teamNumber      = agent->getTeamNumber();
  auto const& ambulator          = agent->getAmbulator();
  auto const& motionScriptModule = agent->getMotionScriptModule();
  auto const& headModule         = agent->getHeadModule();
  auto const& fallDetector       = agent->getFallDetector();

  auto isPenalised = [teamNumber,uniformNumber]()
  {
    auto gameState = State::get<GameState>();
    return gameState && gameState->teamInfo(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto isNotPenalised = [teamNumber,uniformNumber]()
  {
    auto gameState = State::get<GameState>();
    return gameState && !gameState->teamInfo(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto nonPenalisedPlayMode = [isNotPenalised](PlayMode playMode)
  {
    return [isNotPenalised,playMode]()
    {
      auto gameState = State::get<GameState>();
      return gameState && isNotPenalised() && gameState->getPlayMode() == playMode;
    };
  };

  auto isWalking = [ambulator]() { return ambulator->isRunning(); };

  auto hasFallenForward = [fallDetector]() { return fallDetector->getFallenState() == FallState::FORWARD; };

  auto hasFallenBackward = [fallDetector]() { return fallDetector->getFallenState() == FallState::BACKWARD; };

  auto isAgentShutdownRequested = changedTo(true, [agent]() { return agent->isStopRequested(); });

  // BUILD TREE

  auto tree = make_shared<OptionTree>();

  // OPTIONS

  auto sit = make_shared<MotionScriptOption>("sitDownScript", motionScriptModule, "./motionscripts/sit-down.json");
  auto sitArmsBack = make_shared<MotionScriptOption>("sitDownScript", motionScriptModule, "./motionscripts/sit-down-arms-back.json");
  auto standUp = make_shared<MotionScriptOption>("standUpScript", motionScriptModule, "./motionscripts/stand-ready-upright.json");
  auto forwardGetUp = make_shared<MotionScriptOption>("forwardGetUpScript", motionScriptModule, "./motionscripts/get-up-from-front.json");
  auto backwardGetUp = make_shared<MotionScriptOption>("backwardGetUpScript", motionScriptModule, "./motionscripts/get-up-from-back.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", ambulator);

  auto performRole = make_shared<DispatchOption<PlayerRole>>("performRole", [agent](){ return agent->getBehaviourControl()->getPlayerRole(); });
  performRole->setOption(PlayerRole::Keeper, buildKeeperFsm(agent, tree));
  performRole->setOption(PlayerRole::Striker, buildStrikerFsm(agent, tree));
  performRole->setOption(PlayerRole::Supporter, buildSupporterFsm(agent, tree));
  // NOTE for now we re-use the same striker behaviour for the regular striker as the penalty striker
  auto strikerFsm = buildPenaltyKeeperFsm(agent, tree);
  performRole->setOption(PlayerRole::PenaltyStriker, strikerFsm);
  performRole->setOption(PlayerRole::PenaltyKeeper, strikerFsm);

  //
  // Build the top-level FSM
  //

  auto winFsm = make_shared<FSMOption>(agent->getVoice(), "win");

  // The win FSM is the root of the option tree
  tree->addOption(winFsm, /*isRoot*/ true);

  auto startUpState = winFsm->newState("startUp", {sit}, false/*end state*/, true/* start state */);
  auto readyState = winFsm->newState("ready", {stopWalking});
  auto pausing1State = winFsm->newState("pausing1", {stopWalking});
  auto pausing2State = winFsm->newState("pausing2", {sit});
  auto pausedState = winFsm->newState("paused", {});
  auto unpausingState = winFsm->newState("unpausing", {standUp});
  auto setState = winFsm->newState("set", {stopWalking});
  auto playingState = winFsm->newState("playing", {performRole});
  auto penalizedState = winFsm->newState("penalized", {stopWalking});
  auto forwardGetUpState = winFsm->newState("forwardGetUp", {forwardGetUp});
  auto backwardGetUpState = winFsm->newState("backwardGetUp", {backwardGetUp});
  auto stopWalkingForShutdownState = winFsm->newState("stopWalkingForShutdown", {stopWalking});
  auto sitForShutdownState = winFsm->newState("sitForShutdown", {sitArmsBack});
  auto stopAgentAndExitState = winFsm->newState("stopAgentAndExit", {});

  // In the Win FSM, any state other than 'playing' corresponds to the 'waiting' activity.
  setPlayerActivityInStates(agent,
    PlayerActivity::Waiting,
    { startUpState, readyState, pausing1State, pausing2State, pausedState,
      unpausingState, setState, penalizedState, forwardGetUpState,
      backwardGetUpState, stopWalkingForShutdownState, sitForShutdownState,
      stopAgentAndExitState });

  // In the Win FSM, any state other than 'playing' and 'penalised' corresponds to the 'inactive' status.
  setPlayerStatusInStates(agent,
    PlayerStatus::Inactive,
    { startUpState, readyState, pausing1State, pausing2State, pausedState,
      unpausingState, setState, forwardGetUpState,
      backwardGetUpState, stopWalkingForShutdownState, sitForShutdownState,
      stopAgentAndExitState });
  setPlayerStatusInStates(agent, PlayerStatus::Active, { playingState });
  setPlayerStatusInStates(agent, PlayerStatus::Penalised, { penalizedState });

  auto const& debugger = agent->getDebugger();
  readyState->onEnter.connect([debugger,headModule]() { debugger->showReady(); headModule->moveToHome(); });
  setState->onEnter.connect([debugger,headModule]() { debugger->showSet(); headModule->moveToHome(); });
  playingState->onEnter.connect([debugger]() { debugger->showPlaying(); });
  penalizedState->onEnter.connect([debugger,headModule]() { debugger->showPenalized(); headModule->moveToHome(); });
  pausedState->onEnter.connect([debugger]() { debugger->showPaused(); });
  pausing1State->onEnter.connect([debugger,headModule]() { debugger->showPaused(); headModule->moveToHome(); });
  stopAgentAndExitState->onEnter.connect([agent]() { agent->stop(); });

  //
  // START UP
  //

  startUpState
    ->transitionTo(readyState, "initialised")
    ->whenTerminated();

  //
  // PAUSE BUTTON
  //

  pausedState
    ->transitionTo(unpausingState, "button2")
    ->when(startButtonPressed);

  unpausingState
    ->transitionTo(setState, "done")
    ->whenTerminated();

  playingState
    ->transitionTo(pausing1State, "button2")
    ->when(startButtonPressed);

  pausing1State
    ->transitionTo(pausing2State, "stop-walk")
    ->when(negate(isWalking));

  pausing2State
    ->transitionTo(pausedState, "done")
    ->whenTerminated();

  //
  // PLAY MODE BUTTON
  //

  readyState
    ->transitionTo(setState, "button1")
    ->when(modeButtonPressed);

  setState
    ->transitionTo(penalizedState, "button1")
    ->when(modeButtonPressed);

  penalizedState
    ->transitionTo(playingState, "button1")
    ->when(modeButtonPressed);

  //
  // GAME CONTROLLER PLAY MODE
  //

  readyState
    ->transitionTo(setState, "gc-set")
    ->when(isSetPlayMode);

  readyState
    ->transitionTo(playingState, "gc-playing")
    ->when(isPlayingPlayMode);

  setState
    ->transitionTo(penalizedState, "gc-penalised")
    ->when(isPenalised);

  setState
    ->transitionTo(playingState, "gc-playing")
    ->when(isPlayingPlayMode);

  playingState
    ->transitionTo(penalizedState, "gc-penalised")
    ->when(isPenalised);

  playingState
    ->transitionTo(readyState, "gc-ready")
    ->when(nonPenalisedPlayMode(PlayMode::READY));

  playingState
    ->transitionTo(setState, "gc-set")
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  penalizedState
    ->transitionTo(setState, "gc-unpenalised")
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  penalizedState
    ->transitionTo(playingState, "gc-unpenalised")
    ->when(nonPenalisedPlayMode(PlayMode::PLAYING));

  // FALLEN TRANSITIONS
  playingState
    ->transitionTo(forwardGetUpState, "fall-fwd")
    ->when(hasFallenForward);

  playingState
    ->transitionTo(backwardGetUpState, "fall-back")
    ->when(hasFallenBackward);

  forwardGetUpState
    ->transitionTo(playingState, "done")
    ->whenTerminated();

  backwardGetUpState
    ->transitionTo(playingState, "done")
    ->whenTerminated();

  //
  // SHUTDOWN
  //

  // TODO express this sequence more elegantly

  winFsm
    ->wildcardTransitionTo(stopWalkingForShutdownState, "shutdown-request")
    ->when(isAgentShutdownRequested);

  stopWalkingForShutdownState
    ->transitionTo(sitForShutdownState, "stopped")
    ->when(negate(isWalking)); // TODO why can't this be whenTerminated() -- doesn't seem to work (here and in other places)

  sitForShutdownState
    ->transitionTo(stopAgentAndExitState, "sitting")
    ->whenTerminated();

  ofstream winOut("fsm-win.dot");
  winOut << winFsm->toDot();

  return tree;
}

