#pragma once

#include "../option.hh"
#include <string>

namespace bold
{
  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::string const& actionName);

    virtual double hasTerminated() override;

    virtual OptionList runPolicy() override;

  private:
    std::string d_actionName;
    bool d_started;
  };

  inline ActionOption::ActionOption(std::string const& id, std::string const& actionName)
    : Option(id),
      d_actionName(actionName),
      d_started(false)
  {}
}
