#pragma once

#include "../motionmodule.hh"

#include <stdio.h>
#include <string>
#include <set>

#include "../../Control/control.hh"

namespace bold
{
  class MotionScript;
  class MotionScriptRunner;
  class BodySection;

  class MotionScriptModule : public MotionModule
  {
  private:
    std::shared_ptr<MotionScriptRunner> d_runner;
    std::vector<std::shared_ptr<Control const>> d_controls;

  public:
    MotionScriptModule(std::shared_ptr<MotionTaskScheduler> scheduler, std::vector<std::shared_ptr<MotionScript>> scripts);

    void initialize() override;

    void step(std::shared_ptr<JointSelection> selectedJoints) override;

    void applyHead(std::shared_ptr<HeadSection> head) override;
    void applyArms(std::shared_ptr<ArmSection> arms) override;
    void applyLegs(std::shared_ptr<LegSection> legs) override;

    void applySection(std::shared_ptr<BodySection> section);

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; };

    bool start(std::shared_ptr<MotionScriptRunner> scriptRunner);

    bool isRunning();
  };
}
