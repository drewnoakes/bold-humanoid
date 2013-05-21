#pragma once

#include <memory>

#include "../OptionTree/optiontree.hh"

class minIni;

namespace bold
{
  class OptionTreeBuilder
  {
  public:
    virtual std::unique_ptr<OptionTree> buildTree(minIni const& ini) = 0;
  };
}
