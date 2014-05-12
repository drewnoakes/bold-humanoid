#pragma once

#include "../option.hh"

namespace bold
{
  class Agent;
  class MotionScriptOption;

  class GetUpOption : public Option
  {
  public:
    GetUpOption(std::string const& id, Agent* agent);

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override;

    virtual double hasTerminated() override;

  private:
    Agent* d_agent;
    std::shared_ptr<MotionScriptOption> d_activeScript;

    std::shared_ptr<MotionScriptOption> d_forwardGetUp;
    std::shared_ptr<MotionScriptOption> d_backwardGetUp;
    std::shared_ptr<MotionScriptOption> d_rollLeftToFront;
    std::shared_ptr<MotionScriptOption> d_rollRightToFront;
  };
}
