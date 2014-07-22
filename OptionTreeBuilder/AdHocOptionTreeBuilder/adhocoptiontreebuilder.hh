#pragma once

#include "../optiontreebuilder.hh"

namespace bold
{
  class Agent;

  class AdHocOptionTreeBuilder
  {
  public:
    std::shared_ptr<OptionTree> buildTree(Agent* agent);

  private:
    std::shared_ptr<FSMOption> buildPauseStateFsm(Agent* agent, std::shared_ptr<Option> whileUnpausedOption);
    std::shared_ptr<FSMOption> buildStayStandingFsm(Agent* agent, std::shared_ptr<Option> whileStandingOption);
    std::shared_ptr<FSMOption> buildPlayModeFsm(Agent* agent, std::shared_ptr<Option> whilePlayingOption);

    std::shared_ptr<FSMOption> buildStrikerFsm(Agent* agent);
    std::shared_ptr<FSMOption> buildSupporterFsm(Agent* agent);
    std::shared_ptr<FSMOption> buildKeeperFsm(Agent* agent);
    std::shared_ptr<FSMOption> buildPenaltyKeeperFsm(Agent* agent);
    std::shared_ptr<FSMOption> buildKickLearnerFsm(Agent* agent);
    std::shared_ptr<FSMOption> buildBallCirclerFsm(Agent* agent);
  };
}
