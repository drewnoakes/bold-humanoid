#include "adhocoptiontreebuilder.hh"

#include "../../Option/GetUpOption/getupoption.hh"
#include "../../Option/SequenceOption/sequenceoption.hh"
#include "../../Option/StopWalking/stopwalking.hh"
#include "../../StateObject/GameState/gamestate.hh"
#include "../../StateObserver/FallDetector/falldetector.hh"
#include "conditionals.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

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

  setPlayerStatusInStates(agent, PlayerStatus::Active, { standingState });
  setPlayerStatusInStates(agent, PlayerStatus::Inactive, { getUpState });

  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { getUpState });

  // TRANSITIONS

  fsm
    ->wildcardTransitionTo(getUpState, "fallen")
    ->when(hasFallen);

  getUpState
    ->transitionTo(standingState, "done")
    ->whenTerminated();

  return fsm;
}
