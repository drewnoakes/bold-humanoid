#pragma once

#include "../option.hh"
#include "../../MotionScript/motionscript.hh"

#include <string>

namespace bold
{
  class MotionScriptModule;
  class MotionScriptRunner;

  class MotionScriptOption : public Option
  {
  public:
    MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::string const& fileName);

    MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::shared_ptr<MotionScript const> script);

    virtual double hasTerminated() override;

    virtual std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<MotionScriptModule> d_motionScriptModule;
    /// The script associated with this MotionScriptfOption
    std::shared_ptr<MotionScript const> d_script;
    /// The most recent MotionScriptRunner issued to the MotionScriptModule
    std::shared_ptr<MotionScriptRunner> d_runner;
  };

  inline MotionScriptOption::MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::string const& fileName)
    : Option(id),
      d_motionScriptModule(motionScriptModule)
  {
    d_script = MotionScript::fromFile(fileName);
    if (!d_script)
      throw std::runtime_error("File not found");
  }

  inline MotionScriptOption::MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::shared_ptr<MotionScript const> script)
    : Option(id),
      d_motionScriptModule(motionScriptModule),
      d_script(script)
  {}
}
