#pragma once

#include "../optiontreebuilder.hh"

namespace bold
{
  class Agent;

  class AdHocOptionTreeBuilder
  {
  public:
    std::shared_ptr<OptionTree> buildTree(Agent* agent);
  };
}
