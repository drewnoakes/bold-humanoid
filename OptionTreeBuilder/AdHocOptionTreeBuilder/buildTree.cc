#include "adhocoptiontreebuilder.ih"

#include "../../Option/UntilShutdown/untilshutdown.hh"

shared_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(Agent* agent)
{
  // OPTIONS

  auto sitArmsBack = make_shared<MotionScriptOption>("sitDownScript", agent->getMotionScriptModule(), "./motionscripts/sit-down-arms-back.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getWalkModule());
  auto stopAgent = make_shared<ActionOption>("stopAgent", [agent] { agent->stop(); });
  auto shutdownSequence = SequenceOption::make("shutdown-sequence", { stopWalking, sitArmsBack, stopAgent });

  auto keeperFsm = buildKeeperFsm(agent);
  auto strikerFsm = buildStrikerFsm(agent);
  auto supporterFsm = buildSupporterFsm(agent);
  auto penaltyKeeperFsm = buildPenaltyKeeperFsm(agent);
  auto kickLearnerFsm = buildKickLearnerFsm(agent);

  auto performRole = make_shared<DispatchOption<PlayerRole>>("performRole", [agent] { return agent->getBehaviourControl()->getPlayerRole(); });
  performRole->setOption(PlayerRole::Keeper, keeperFsm);
  performRole->setOption(PlayerRole::Striker, strikerFsm);
  performRole->setOption(PlayerRole::Supporter, supporterFsm);
  performRole->setOption(PlayerRole::PenaltyStriker, strikerFsm); // NOTE reusing regular striker for now
  performRole->setOption(PlayerRole::PenaltyKeeper, penaltyKeeperFsm);
  performRole->setOption(PlayerRole::KickLearner, kickLearnerFsm);

  // Build the top-level FSM

  auto stayStanding = buildStayStandingFsm(agent, performRole);
  auto respectPlayMode = buildPlayModeFsm(agent, stayStanding);
  auto allowPause = buildPauseStateFsm(agent, respectPlayMode);

  auto untilShutdown = make_shared<UntilShutdown>("until-shutdown", agent, allowPause, shutdownSequence);

  // BUILD TREE

  auto tree = make_shared<OptionTree>();

  // Register all FSMs with the tree so that we can debug them via Round Table
  tree->addOption(untilShutdown, /* root */ true);
  tree->addOption(allowPause, false);
  tree->addOption(stayStanding, false);
  tree->addOption(respectPlayMode, false);
  tree->addOption(keeperFsm, false);
  tree->addOption(strikerFsm, false);
  tree->addOption(supporterFsm, false);
  tree->addOption(penaltyKeeperFsm, false);
  tree->addOption(kickLearnerFsm, false);

  return tree;
}
