#ifndef BOLD_LOOKATBALL_HH
#define BOLD_LOOKATBALL_HH

#include "../option.hh"

namespace bold
{
  class LookAtBall : public Option
  {
  public:
    OptionPtr runPolicy();
  };

}

#endif
