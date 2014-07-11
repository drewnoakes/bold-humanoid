#pragma once

#include "../option.hh"

namespace bold
{
  class Agent;

  // TODO would a generic 'until' option that takes a std::function be useful?

  /** Executes the specified sub-option until the agent shuts down. */
  class UntilShutdown : public Option
  {
  public:
    UntilShutdown(std::string id, Agent* agent, std::shared_ptr<Option> beforeShutdown, std::shared_ptr<Option> afterShutdown);

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    Agent* d_agent;
    std::shared_ptr<Option> d_beforeShutdown;
    std::shared_ptr<Option> d_afterShutdown;
  };
}
