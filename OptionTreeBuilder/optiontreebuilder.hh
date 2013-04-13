#ifndef BOLD_OPTIONTREEBUILDER_HH
#define BOLD_OPTIONTREEBUILDER_HH

#include "../OptionTree/optiontree.hh"
#include <memory>
#include <minIni.h>

namespace bold
{
  class OptionTreeBuilder
  {
  public:
    virtual std::unique_ptr<OptionTree> buildTree(minIni const& ini) = 0;
  };

}

#endif
