#pragma once

#include "../motionmodule.hh"

#include <stdio.h>
#include <string>
#include <set>

namespace bold
{
  class MotionScript;
  class MotionScriptRunner;
  class BodySection;

  class MotionScriptModule : public MotionModule
  {
  private:
    std::shared_ptr<MotionScriptRunner> d_runner;

  public:
    MotionScriptModule(std::shared_ptr<MotionTaskScheduler> scheduler, std::vector<std::shared_ptr<MotionScript>> scripts);
    ~MotionScriptModule() override;

    MotionScriptModule(const MotionScriptModule&) = delete;
    MotionScriptModule& operator=(const MotionScriptModule&) = delete;

    void initialize() override;

    void step(std::shared_ptr<JointSelection> selectedJoints) override;

    void applyHead(std::shared_ptr<HeadSection> head) override;
    void applyArms(std::shared_ptr<ArmSection> arms) override;
    void applyLegs(std::shared_ptr<LegSection> legs) override;

    void applySection(std::shared_ptr<BodySection> section);

    /** Attempts to start executing the provided script runner.
     *
     * @return true if the script has been accepted and scheduled to run,
     * otherwise false indicating that the request to start was ignored due
     * to another script already executing.
     */
    bool start(std::shared_ptr<MotionScriptRunner> scriptRunner);

    bool isRunning();
  };
}
