#ifndef BOLD_CIRCLE_HH
#define BOLD_CIRCLE_HH

#include "../option.hh"


namespace bold
{
  class Circle : public Option
  {
    Circle(std::string const& id) : Option(id) {}

    virtual OptionList runPolicy() override;
  };
}

#endif
