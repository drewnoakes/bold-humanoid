#pragma once

#include "../motionmodule.hh"

#include <stdio.h>
#include <string>
#include <set>

#include "../../Control/control.hh"

namespace bold
{
  class MotionScriptFile;
  class MotionScriptPage;
  class MotionScriptRunner;
  class BodySection;

  class ActionModule : public MotionModule
  {
  private:
    std::shared_ptr<MotionScriptFile> d_file;
    std::shared_ptr<MotionScriptRunner> d_runner;

    std::vector<std::shared_ptr<Control const>> d_controls;

    bool start(int index, std::shared_ptr<MotionScriptPage> page);

  public:
    ActionModule(std::shared_ptr<MotionTaskScheduler> scheduler, std::shared_ptr<MotionScriptFile> file);

    void initialize() override;

    void step(std::shared_ptr<JointSelection> selectedJoints) override;

    void applyHead(std::shared_ptr<HeadSection> head) override;
    void applyArms(std::shared_ptr<ArmSection> arms) override;
    void applyLegs(std::shared_ptr<LegSection> legs) override;

    void applySection(std::shared_ptr<BodySection> section);

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; };

    // TODO replace start with: run(shared_ptr<MotionScriptRunner>)

    bool start(int pageIndex);

    /** Play the first page having the specified name. */
    bool start(std::string const& pageName);

    bool isRunning();
  };
}
