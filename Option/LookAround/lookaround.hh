#ifndef BOLD_LOOKAROUND_HH
#define BOLD_LOOKAROUND_HH

#include "../option.hh"

namespace bold
{
  class LookAround : public Option
  {
  public:
    LookAround(std::string const& id) : Option(id) {}

    virtual OptionPtr runPolicy() override;
  };
}

#endif
