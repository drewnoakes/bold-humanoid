#include "adhocoptiontreebuilder.ih"

#include "../../StateObserver/ButtonObserver/buttonobserver.hh"

shared_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(Agent* agent)
{
  uchar uniformNumber   = agent->getUniformNumber();
  uchar teamNumber      = agent->getTeamNumber();
  auto const& walkModule         = agent->getWalkModule();
  auto const& motionScriptModule = agent->getMotionScriptModule();
  auto const& headModule         = agent->getHeadModule();
  auto const& fallDetector       = agent->getFallDetector();

  auto isPenalised = [teamNumber,uniformNumber]()
  {
    auto gameState = State::get<GameState>();
    return gameState && gameState->getTeam(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto isNotPenalised = [teamNumber,uniformNumber]()
  {
    auto gameState = State::get<GameState>();
    return gameState && !gameState->getTeam(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto nonPenalisedPlayMode = [isNotPenalised](PlayMode playMode)
  {
    return [isNotPenalised,playMode]()
    {
      auto gameState = State::get<GameState>();
      return gameState && isNotPenalised() && gameState->getPlayMode() == playMode;
    };
  };

  auto isWalking = [walkModule]() { return walkModule->isRunning(); };
  auto hasFallen = [fallDetector]() { return fallDetector->getFallenState() != FallState::STANDUP; };

  auto isAgentShutdownRequested = changedTo(true, [agent]() { return agent->isStopRequested(); });

  // BUILD TREE

  auto tree = make_shared<OptionTree>();

  // OPTIONS

  auto sit = make_shared<MotionScriptOption>("sitDownScript", motionScriptModule, "./motionscripts/sit-down.json");
  auto sitArmsBack = make_shared<MotionScriptOption>("sitDownScript", motionScriptModule, "./motionscripts/sit-down-arms-back.json");
  auto standUp = make_shared<MotionScriptOption>("standUpScript", motionScriptModule, "./motionscripts/stand-ready-upright.json");
  auto getUp = make_shared<GetUpOption>("getUp", agent);
  auto stopWalking = make_shared<StopWalking>("stopWalking", walkModule);
  auto stopWalkingImmediately = make_shared<StopWalking>("stopWalking", walkModule, /*immediately*/ true);
  auto stopAgent = make_shared<ActionOption>("stopAgent", [agent]() { agent->stop(); });

  auto performRole = make_shared<DispatchOption<PlayerRole>>("performRole", [agent](){ return agent->getBehaviourControl()->getPlayerRole(); });
  performRole->setOption(PlayerRole::Keeper, buildKeeperFsm(agent, tree));
  // NOTE for now we re-use the same striker behaviour for the regular striker as the penalty striker
  auto strikerFsm = buildStrikerFsm(agent, tree);
  performRole->setOption(PlayerRole::Striker, strikerFsm);
  performRole->setOption(PlayerRole::Supporter, buildSupporterFsm(agent, tree));
  performRole->setOption(PlayerRole::PenaltyStriker, strikerFsm);
  performRole->setOption(PlayerRole::PenaltyKeeper, buildPenaltyKeeperFsm(agent, tree));
  performRole->setOption(PlayerRole::KickLearner, buildKickLearnerFsm(agent, tree));

  //
  // Build the top-level FSM
  //

  auto winFsm = make_shared<FSMOption>(agent->getVoice(), "win");

  // The win FSM is the root of the option tree
  tree->addOption(winFsm, /*isRoot*/ true);

  auto startUpState = winFsm->newState("startUp", {sit}, false/*end state*/, true/* start state */);
  auto readyState = winFsm->newState("ready", {stopWalking});
  auto pauseState = winFsm->newState("pause", {SequenceOption::make("pause-sequence", {stopWalking,sit})});
  auto setState = winFsm->newState("set", {SequenceOption::make("pause-sequence", {stopWalking,standUp})});
  auto playingState = winFsm->newState("playing", {performRole});
  auto penalisedState = winFsm->newState("penalised", {stopWalking});
  auto getUpState = winFsm->newState("getUp", {SequenceOption::make("get-up-sequence",  {stopWalkingImmediately,getUp})});
  auto shutdownState = winFsm->newState("shutdown", {SequenceOption::make("shutdown-sequence", {stopWalking,sitArmsBack,stopAgent})});

  // In the Win FSM, any state other than 'playing' corresponds to the 'waiting' activity.
  setPlayerActivityInStates(agent,
    PlayerActivity::Waiting,
    {
      startUpState, readyState, pauseState, setState, penalisedState,
      getUpState,
      shutdownState
    });

  // In the Win FSM, any state other than 'playing' and 'penalised' corresponds to the 'inactive' status.
  setPlayerStatusInStates(agent,
    PlayerStatus::Inactive,
    {
      startUpState, readyState, pauseState,
      setState,
      getUpState,
      shutdownState
    });

  setPlayerStatusInStates(agent, PlayerStatus::Active, { playingState });
  setPlayerStatusInStates(agent, PlayerStatus::Penalised, { penalisedState });
  setPlayerStatusInStates(agent, PlayerStatus::Paused, { pauseState });

  auto const& debugger = agent->getDebugger();
  readyState->onEnter.connect([debugger,headModule]() { debugger->showReady(); headModule->moveToHome(); });
  setState->onEnter.connect([debugger,headModule]() { debugger->showSet(); headModule->moveToHome(); });
  playingState->onEnter.connect([debugger]() { debugger->showPlaying(); });
  penalisedState->onEnter.connect([debugger,headModule]() { debugger->showPenalised(); headModule->moveToHome(); });
  pauseState->onEnter.connect([debugger,headModule]() { debugger->showPaused(); headModule->moveToHome(); });

  //
  // START UP
  //

  startUpState
    ->transitionTo(readyState, "initialised")
    ->whenTerminated();

  //
  // PAUSE BUTTON
  //

  shared_ptr<ButtonTracker> pauseButton = agent->getButtonObserver()->track(Button::Middle);

  winFsm
    ->wildcardTransitionTo(pauseState, "middle-button")
    ->when([pauseButton] { return pauseButton->isPressedForMillis(200); });

  pauseState
    ->transitionTo(setState, "middle-button")
    ->when([pauseButton] { return pauseButton->isPressedForMillis(80); });

  //
  // PLAY MODE BUTTON
  //

  shared_ptr<ButtonTracker> modeButton = agent->getButtonObserver()->track(Button::Left);

  readyState
    ->transitionTo(setState, "left-button")
    ->when([modeButton] { return modeButton->isPressedForMillis(80); });

  setState
    ->transitionTo(playingState, "left-button")
    ->when([modeButton] { return modeButton->isPressedForMillis(80); });

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
    ->transitionTo(penalisedState, "gc-penalised")
    ->when(isPenalised);

  setState
    ->transitionTo(playingState, "gc-playing")
    ->when(isPlayingPlayMode);

  playingState
    ->transitionTo(penalisedState, "gc-penalised")
    ->when(isPenalised);

  playingState
    ->transitionTo(readyState, "gc-ready")
    ->when(nonPenalisedPlayMode(PlayMode::READY));

  playingState
    ->transitionTo(setState, "gc-set")
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  penalisedState
    ->transitionTo(setState, "gc-unpenalised")
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  penalisedState
    ->transitionTo(playingState, "gc-unpenalised")
    ->when(nonPenalisedPlayMode(PlayMode::PLAYING));

  //
  // GET UP FROM FALL
  //

  playingState
    ->transitionTo(getUpState, "fallen")
    ->when(hasFallen);

  getUpState
    ->transitionTo(playingState, "done")
    ->whenTerminated();

  //
  // SHUTDOWN
  //

  winFsm
    ->wildcardTransitionTo(shutdownState, "shutdown-request")
    ->when(isAgentShutdownRequested);

  ofstream winOut("fsm-win.dot");
  winOut << winFsm->toDot();

  return tree;
}

