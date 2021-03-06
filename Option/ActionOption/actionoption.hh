#pragma once

#include "../option.hh"
#include <functional>

namespace bold
{
  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::function<void()> action)
    : Option(id, "Action"),
      d_action(action),
      d_needsRunning(true)
    {}

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void reset() override;

    double hasTerminated() override;

  private:
    std::function<void()> d_action;
    bool d_needsRunning;
  };
}
