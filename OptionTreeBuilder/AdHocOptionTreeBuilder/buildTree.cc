#include "adhocoptiontreebuilder.hh"

#include "../../BehaviourControl/behaviourcontrol.hh"
#include "../../Option/ActionOption/actionoption.hh"
#include "../../Option/DispatchOption/dispatchoption.hh"
#include "../../Option/FSMOption/fsmoption.hh"
#include "../../Option/MotionScriptOption/motionscriptoption.hh"
#include "../../Option/SequenceOption/sequenceoption.hh"
#include "../../Option/StopWalking/stopwalking.hh"
#include "../../Option/UntilShutdown/untilshutdown.hh"

using namespace bold;
using namespace std;

shared_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(Agent* agent)
{
  // OPTIONS

  auto sitArmsBack = make_shared<MotionScriptOption>("sit-down-script", agent->getMotionScriptModule(), "./motionscripts/sit-down-arms-back.json");
  auto sit = make_shared<MotionScriptOption>("sit-down-script", agent->getMotionScriptModule(), "./motionscripts/sit-down.json");
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto stopAgent = make_shared<ActionOption>("stop-agent", [agent] { agent->stop(); });
  auto shutdownSequence = SequenceOption::make("shutdown-sequence", { stopWalking, sitArmsBack, stopAgent });

  auto keeperFsm = buildKeeperFsm(agent);
  auto strikerFsm = buildStrikerFsm(agent);
  auto supporterFsm = buildSupporterFsm(agent);
  auto penaltyKeeperFsm = buildPenaltyKeeperFsm(agent);
  auto kickLearnerFsm = buildKickLearnerFsm(agent);
  auto ballCirclerFsm = buildBallCirclerFsm(agent);
  auto whistleListenerFsm = buildWhistleListenerFsm(agent);

  auto performRole = make_shared<DispatchOption<PlayerRole>>("perform-role", [agent] { return agent->getBehaviourControl()->getPlayerRole(); });
  performRole->setOption(PlayerRole::Keeper, keeperFsm);
  performRole->setOption(PlayerRole::Striker, strikerFsm);
  performRole->setOption(PlayerRole::Supporter, supporterFsm);
  performRole->setOption(PlayerRole::PenaltyStriker, strikerFsm); // NOTE reusing regular striker for now
  performRole->setOption(PlayerRole::PenaltyKeeper, penaltyKeeperFsm);
  performRole->setOption(PlayerRole::KickLearner, kickLearnerFsm);
  performRole->setOption(PlayerRole::BallCircler, ballCirclerFsm);
  performRole->setOption(PlayerRole::WhistleListener, whistleListenerFsm);

  // Build the top-level FSM

  auto stayStandingFsm = buildStayStandingFsm(agent, performRole);
  auto respectPlayModeFsm = buildPlayModeFsm(agent, stayStandingFsm);
  auto allowPauseFsm = buildPauseStateFsm(agent, respectPlayModeFsm);

  auto untilShutdown = make_shared<UntilShutdown>("until-shutdown", agent, allowPauseFsm, shutdownSequence);

  // BUILD TREE

  auto tree = make_shared<OptionTree>(SequenceOption::make("boot", { sit, untilShutdown }));

  // Register all FSMs with the tree so that we can debug them via Round Table
  tree->registerFsm(allowPauseFsm);
  tree->registerFsm(stayStandingFsm);
  tree->registerFsm(respectPlayModeFsm);
  tree->registerFsm(keeperFsm);
  tree->registerFsm(strikerFsm);
  tree->registerFsm(supporterFsm);
  tree->registerFsm(penaltyKeeperFsm);
  tree->registerFsm(kickLearnerFsm);
  tree->registerFsm(ballCirclerFsm);
  tree->registerFsm(whistleListenerFsm);

  return tree;
}
