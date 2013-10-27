#pragma once

#include "../option.hh"
#include "../../MotionScript/motionscript.hh"

#include <string>

namespace bold
{
  class ActionModule;
  class MotionScriptRunner;

  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::shared_ptr<ActionModule> actionModule, std::string const& fileName);

    ActionOption(std::string const& id, std::shared_ptr<ActionModule> actionModule, std::shared_ptr<MotionScript const> script);

    virtual double hasTerminated() override;

    virtual std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<ActionModule> d_actionModule;
    std::shared_ptr<MotionScript const> d_script;
    std::shared_ptr<MotionScriptRunner> d_runner;
  };

  inline ActionOption::ActionOption(std::string const& id, std::shared_ptr<ActionModule> actionModule, std::string const& fileName)
    : Option(id),
      d_actionModule(actionModule)
  {
    d_script = MotionScript::fromFile(fileName);
    if (!d_script)
      throw std::runtime_error("File not found");
  }

  inline ActionOption::ActionOption(std::string const& id, std::shared_ptr<ActionModule> actionModule, std::shared_ptr<MotionScript const> script)
    : Option(id),
      d_actionModule(actionModule),
      d_script(script)
  {}
}
