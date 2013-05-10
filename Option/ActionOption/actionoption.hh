#pragma once

#include "../option.hh"
#include <string>

namespace bold
{
  class Action;

  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::string const& actionName, std::shared_ptr<Action> actionModule);

    virtual double hasTerminated() override;

    virtual OptionList runPolicy() override;

  private:
    std::shared_ptr<Action> d_actionModule;
    std::string d_actionName;
    bool d_started;
  };

  inline ActionOption::ActionOption(std::string const& id, std::string const& actionName, std::shared_ptr<Action> actionModule)
    : Option(id),
      d_actionModule(actionModule),
      d_actionName(actionName),
      d_started(false)
  {}
}
