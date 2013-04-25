#ifndef BOLD_ADHOCOPTIONTREEBUILDER_HH
#define BOLD_ADHOCOPTIONTREEBUILDER_HH

#include "../optiontreebuilder.hh"

namespace bold
{
  class Debugger;

  class AdHocOptionTreeBuilder
  {
  public:
    std::unique_ptr<OptionTree> buildTree(minIni const& ini,
                                          unsigned teamNumber,
                                          unsigned uniformNumber,
                                          bool ignoreGameController,
                                          std::shared_ptr<Debugger> debugger);
  };
}

#endif
