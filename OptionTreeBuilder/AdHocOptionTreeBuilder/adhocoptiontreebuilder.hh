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
    std::shared_ptr<FSMOption> buildStrikerFsm(Agent* agent, std::shared_ptr<OptionTree> tree);
    std::shared_ptr<FSMOption> buildSupporterFsm(Agent* agent, std::shared_ptr<OptionTree> tree);
    std::shared_ptr<FSMOption> buildKeeperFsm(Agent* agent, std::shared_ptr<OptionTree> tree);
    std::shared_ptr<FSMOption> buildPenaltyKeeperFsm(Agent* agent, std::shared_ptr<OptionTree> tree);
  };
}
