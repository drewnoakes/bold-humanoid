#pragma once

#include "../option.hh"
#include <string>

namespace bold
{
  enum class ActionPage
  {
    ForwardGetUp = 10,
    BackwardGetUp = 11,
    KickRight = 12,
    KickLeft = 13,
    None = 255
  };

  class ActionModule;
  class MotionScriptRunner;

  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::string const& actionName, std::shared_ptr<ActionModule> actionModule);

    ActionOption(std::string const& id, ActionPage actionPage, std::shared_ptr<ActionModule> actionModule);

    virtual double hasTerminated() override;

    virtual std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<ActionModule> d_actionModule;
    std::string d_actionName;
    ActionPage d_actionPage;
    std::shared_ptr<MotionScriptRunner> d_runner;
  };

  inline ActionOption::ActionOption(std::string const& id, std::string const& actionName, std::shared_ptr<ActionModule> actionModule)
    : Option(id),
      d_actionModule(actionModule),
      d_actionName(actionName),
      d_actionPage(ActionPage::None)
  {}

  inline ActionOption::ActionOption(std::string const& id, ActionPage actionPage, std::shared_ptr<ActionModule> actionModule)
    : Option(id),
      d_actionModule(actionModule),
      d_actionName(""),
      d_actionPage(actionPage)
  {}
}
