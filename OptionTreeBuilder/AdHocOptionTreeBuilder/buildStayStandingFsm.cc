#include "adhocoptiontreebuilder.ih"

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildStayStandingFsm(Agent* agent, shared_ptr<Option> whileStandingOption)
{
  // TODO this is probably simple enough to allow making it an Option subclass rather than an FSM

  auto hasFallen = [agent] { return agent->getFallDetector()->getFallenState() != FallState::STANDUP; };

  // OPTIONS

  auto getUp = make_shared<GetUpOption>("getUp", agent);
  auto stopWalkingImmediately = make_shared<StopWalking>("stopWalking", agent->getWalkModule(), /*immediately*/ true);

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "stay-standing");

  auto getUpState = fsm->newState("getUp", { SequenceOption::make("get-up-sequence", { stopWalkingImmediately, getUp }) }, false, true);
  auto standingState = fsm->newState("standing", { whileStandingOption });

  getUpState->onEnter.connect([agent]
  {
    agent->getBehaviourControl()->setPlayerActivity(PlayerActivity::Waiting);
    agent->getBehaviourControl()->setPlayerStatus(PlayerStatus::Inactive);
  });

  // TRANSITIONS

  standingState
    ->transitionTo(getUpState, "fallen")
    ->when(hasFallen);

  getUpState
    ->transitionTo(standingState, "done")
    ->whenTerminated();

  return fsm;
}
