#ifndef BOLD_ADHOCOPTIONTREEBUILDER_HH
#define BOLD_ADHOCOPTIONTREEBUILDER_HH

#include "../optiontreebuilder.hh"

namespace bold
{
  class AdHocOptionTreeBuilder
  {
  public:
    std::unique_ptr<OptionTree> buildTree(minIni const& ini);
  };
}

#endif
