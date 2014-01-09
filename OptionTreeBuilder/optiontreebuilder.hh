#pragma once

#include <memory>

#include "../Agent/agent.hh"
#include "../OptionTree/optiontree.hh"

namespace bold
{
  class OptionTreeBuilder
  {
  public:
    virtual std::unique_ptr<OptionTree> buildTree(Agent* agent) = 0;
  };
}
