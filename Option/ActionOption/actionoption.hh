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


  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::string const& actionName);

    ActionOption(std::string const& id, ActionPage actionPage);

    virtual double hasTerminated() override;

    virtual OptionList runPolicy() override;

  private:
    std::string d_actionName;
    ActionPage d_actionPage;
    bool d_started;
  };

  inline ActionOption::ActionOption(std::string const& id, std::string const& actionName)
    : Option(id),
      d_actionName(actionName),
      d_actionPage(ActionPage::None),
      d_started(false)
  {}

  inline ActionOption::ActionOption(std::string const& id, ActionPage actionPage)
    : Option(id),
      d_actionName(""),
      d_actionPage(actionPage),
      d_started(false)
  {}
}
