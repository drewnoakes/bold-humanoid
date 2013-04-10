#ifndef BOLD_STAND_HH
#define BOLD_STAND_HH

#include "../option.hh"

namespace bold
{
  class Stand : public Option
  {
  public:
    Stand(std::string const& id) : Option(id) {}

    OptionList runPolicy();
  };
}

#endif
