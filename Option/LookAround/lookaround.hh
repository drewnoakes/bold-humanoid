#ifndef BOLD_LOOKAROUND_HH
#define BOLD_LOOKAROUND_HH

#include "../option.hh"

namespace bold
{
  class LookAround : public Option
  {
    virtual OptionPtr runPolicy();
  };

}

#endif
