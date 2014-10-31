#pragma once

#include "../motionmodule.hh"

#include <stdio.h>
#include <string>
#include <set>

namespace bold
{
  class MotionRequest;
  class MotionScript;
  class MotionScriptRunner;

  class MotionScriptModule : public MotionModule
  {
  public:
    static void createActions(std::string const& path, std::shared_ptr<MotionScriptModule> const& module);

    MotionScriptModule(std::shared_ptr<MotionTaskScheduler> scheduler);

    ~MotionScriptModule() override = default;

    void step(std::shared_ptr<JointSelection> const& selectedJoints) override;

    void applyHead(HeadSection* head) override;
    void applyArms(ArmSection* arms) override;
    void applyLegs(LegSection* legs) override;

    /** Attempts to execute the provided script runner.
     */
    std::shared_ptr<MotionRequest const> run(std::shared_ptr<MotionScriptRunner> const& scriptRunner);

    /** Attempts to run the provided script.
     */
    std::shared_ptr<MotionRequest const> run(std::shared_ptr<MotionScript const> const& scriptRunner);

    bool isRunning() const;

  private:
    MotionScriptModule(const MotionScriptModule&) = delete;
    MotionScriptModule& operator=(const MotionScriptModule&) = delete;

    std::shared_ptr<MotionScriptRunner> d_runner;
    std::shared_ptr<MotionRequest const> d_motionRequest;
  };
}
