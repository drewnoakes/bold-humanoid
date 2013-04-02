#ifndef BOLD_ACTIONOPTION_HH
#define BOLD_ACTIONOPTION_HH

#include "../option.hh"
#include <string>

namespace bold
{
  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& actionName);

    virtual double hasTerminated();

    virtual OptionPtr runPolicy();

  private:
    std::string d_actionName;
    bool d_started;
  };

  inline ActionOption::ActionOption(std::string const& actionName)
    : d_actionName(actionName),
      d_started(false)
  {}
}

#endif
