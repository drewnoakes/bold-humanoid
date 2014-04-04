#pragma once

#include "../option.hh"

#include <string>

namespace bold
{
  class MotionScript;
  class MotionScriptModule;
  class MotionScriptRunner;

  class MotionScriptOption : public Option
  {
  public:
    MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::string const& fileName);

    MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::shared_ptr<MotionScript const> script);

    virtual double hasTerminated() override;

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override;

  private:
    std::shared_ptr<MotionScriptModule> d_motionScriptModule;
    /// The script associated with this MotionScriptOption
    std::shared_ptr<MotionScript const> d_script;
    /// The most recent MotionScriptRunner issued to the MotionScriptModule
    std::shared_ptr<MotionScriptRunner> d_runner;
  };
}
