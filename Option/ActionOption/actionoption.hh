#pragma once

#include "../option.hh"
#include <string>

namespace bold
{
  class ActionModule;

  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::string const& actionName, std::shared_ptr<ActionModule> actionModule);

    virtual double hasTerminated() override;

    virtual OptionList runPolicy() override;

  private:
    std::shared_ptr<ActionModule> d_actionModule;
    std::string d_actionName;
    bool d_started;
  };

  inline ActionOption::ActionOption(std::string const& id, std::string const& actionName, std::shared_ptr<ActionModule> actionModule)
    : Option(id),
      d_actionModule(actionModule),
      d_actionName(actionName),
      d_started(false)
  {}
}
