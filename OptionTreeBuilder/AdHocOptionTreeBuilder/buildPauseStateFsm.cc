#include "adhocoptiontreebuilder.ih"

#include "../../StateObserver/ButtonObserver/buttonobserver.hh"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildPauseStateFsm(Agent* agent, shared_ptr<Option> whileUnpausedOption)
{
  // TODO this is probably simple enough to allow making it an Option subclass rather than an FSM

  shared_ptr<ButtonTracker> pauseButton = agent->getButtonObserver()->track(Button::Middle);

  // OPTIONS

  auto sit = make_shared<MotionScriptOption>("sit-down-script", agent->getMotionScriptModule(), "./motionscripts/sit-down.json");
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto stopAndSitSequence = SequenceOption::make("stop-then-sit-sequence", { stopWalking, sit });

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "allow-pause");

  auto unpausedState = fsm->newState("unpaused", { whileUnpausedOption }, false, true);
  auto pausedState = fsm->newState("paused", { stopAndSitSequence });

  pausedState->onEnter.connect([agent]
  {
    agent->getBehaviourControl()->setPlayerActivity(PlayerActivity::Waiting);
    agent->getBehaviourControl()->setPlayerStatus(PlayerStatus::Paused);
    agent->getHeadModule()->moveToHome();
  });

  // TRANSITIONS

  fsm
    ->wildcardTransitionTo(pausedState, "middle-button")
    ->when([pauseButton] { return pauseButton->isPressedForMillis(200); });

  pausedState
    ->transitionTo(unpausedState, "middle-button")
    ->when([pauseButton] { return pauseButton->isPressedForMillis(80); });

  return fsm;
}
