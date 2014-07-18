#include "adhocoptiontreebuilder.ih"

#include "../../Option/GameOver/gameover.hh"
#include "../../StateObserver/ButtonObserver/buttonobserver.hh"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildPlayModeFsm(Agent* agent, shared_ptr<Option> whilePlayingOption)
{
  // OPTIONS

  auto sit = make_shared<MotionScriptOption>("sit-down-script", agent->getMotionScriptModule(), "./motionscripts/sit-down.json", true);
  auto standUp = make_shared<MotionScriptOption>("stand-up-script", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json", true);
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto gameOver = make_shared<GameOver>("game-over", agent->getMotionScriptModule(), agent->getVoice());
  auto stopAndSitSequence = SequenceOption::make("stop-then-sit-sequence", { stopWalking, sit });

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "play-mode");

  auto initialState = fsm->newState("initial", { stopAndSitSequence });
  auto readyState = fsm->newState("ready", { stopAndSitSequence }, false, true);
  auto setState = fsm->newState("set", { SequenceOption::make("pause-sequence", { stopWalking, standUp }) });
  auto playingState = fsm->newState("playing", { whilePlayingOption });
  auto penalisedState = fsm->newState("penalised", { stopAndSitSequence });
  auto unpenalisedState = fsm->newState("unpenalised", { whilePlayingOption });
  auto finishedState = fsm->newState("finished", { SequenceOption::make("stop-then-game-over-sequence", { stopWalking, standUp, gameOver }) });

  // STATUSES

  // In the Win FSM, any state other than 'playing' corresponds to the 'waiting' activity.
  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { initialState, readyState, setState, penalisedState, });

  // In the Win FSM, any state other than 'playing' and 'penalised' corresponds to the 'inactive' status.
  setPlayerStatusInStates(agent, PlayerStatus::Inactive, { initialState, readyState, setState, });

  setPlayerStatusInStates(agent, PlayerStatus::Penalised, { penalisedState });

  initialState    ->onEnter.connect([agent] { agent->getBehaviourControl()->setPlayMode(PlayMode::INITIAL);   agent->getHeadModule()->moveToHome(); });
  readyState      ->onEnter.connect([agent] { agent->getBehaviourControl()->setPlayMode(PlayMode::READY);     agent->getHeadModule()->moveToHome(); });
  setState        ->onEnter.connect([agent] { agent->getBehaviourControl()->setPlayMode(PlayMode::SET);       agent->getHeadModule()->moveToHome(); });
  playingState    ->onEnter.connect([agent] { agent->getBehaviourControl()->setPlayMode(PlayMode::PLAYING); });
  unpenalisedState->onEnter.connect([agent] { agent->getBehaviourControl()->setPlayMode(PlayMode::PLAYING); });
  finishedState   ->onEnter.connect([agent] { agent->getBehaviourControl()->setPlayMode(PlayMode::FINISHED); });
  penalisedState  ->onEnter.connect([agent] {                                                                 agent->getHeadModule()->moveToHome(); });

  // TRANSITIONS

  //
  // PLAY MODE BUTTON
  //

  shared_ptr<ButtonTracker> modeButton = agent->getButtonObserver()->track(Button::Left);

  initialState
    ->transitionTo(readyState, "left-button")
    ->when([modeButton] { return modeButton->isPressedForMillis(80); });

  readyState
    ->transitionTo(setState, "left-button")
    ->when([modeButton] { return modeButton->isPressedForMillis(80); });

  setState
    ->transitionTo(playingState, "left-button")
    ->when([modeButton] { return modeButton->isPressedForMillis(80); });

  penalisedState
    ->transitionTo(unpenalisedState, "left-button")
    ->when([modeButton] { return modeButton->isPressedForMillis(80); });

  finishedState
    ->transitionTo(initialState, "left-button")
    ->when([modeButton] { return modeButton->isPressedForMillis(80); });

  //
  // GAME CONTROLLER PLAY MODE
  //

  fsm
    ->wildcardTransitionTo(initialState, "gc-initial")
    ->when(nonPenalisedPlayMode(PlayMode::INITIAL));

  fsm
    ->wildcardTransitionTo(readyState, "gc-ready")
    ->when(nonPenalisedPlayMode(PlayMode::READY));

  fsm
    ->wildcardTransitionTo(setState, "gc-set")
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  fsm
    ->wildcardTransitionTo(playingState, "gc-playing")
    ->when(nonPenalisedPlayMode(PlayMode::PLAYING));

  fsm
    ->wildcardTransitionTo(penalisedState, "gc-penalised")
    ->when([fsm,unpenalisedState] { return isPenalised() && fsm->getCurrentState() != unpenalisedState; });

  fsm
    ->wildcardTransitionTo(setState, "gc-set")
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  fsm
    ->wildcardTransitionTo(finishedState, "gc-finished")
    ->when(nonPenalisedPlayMode(PlayMode::FINISHED));

  return fsm;
}
