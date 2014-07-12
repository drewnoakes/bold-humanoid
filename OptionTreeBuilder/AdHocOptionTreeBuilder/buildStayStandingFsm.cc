#include "adhocoptiontreebuilder.ih"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildStayStandingFsm(Agent* agent, shared_ptr<Option> whileStandingOption)
{
  // TODO this is probably simple enough to allow making it an Option subclass rather than an FSM

  auto hasFallen = [agent] { return agent->getFallDetector()->getFallenState() != FallState::STANDUP; };

  // OPTIONS

  auto getUp = make_shared<GetUpOption>("get-up", agent);
  auto stopWalkingImmediately = make_shared<StopWalking>("stop-walking", agent->getWalkModule(), /*immediately*/ true);

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "stay-standing");

  auto standingState = fsm->newState("standing", { whileStandingOption }, false, true);
  auto getUpState = fsm->newState("get-up", { SequenceOption::make("get-up-sequence", { stopWalkingImmediately, getUp }) });

  getUpState->onEnter.connect([agent]
  {
    agent->getBehaviourControl()->setPlayerActivity(PlayerActivity::Waiting);
    agent->getBehaviourControl()->setPlayerStatus(PlayerStatus::Inactive);
  });

  // TRANSITIONS

  fsm
    ->wildcardTransitionTo(getUpState, "fallen")
    ->when(hasFallen);

  getUpState
    ->transitionTo(standingState, "done")
    ->whenTerminated();

  return fsm;
}
