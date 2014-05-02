#pragma once

#include "../motionmodule.hh"

#include <stdio.h>
#include <string>
#include <set>

namespace bold
{
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

    /** Attempts to start executing the provided script runner.
     *
     * @return true if the script has been accepted and scheduled to run,
     * otherwise false indicating that the request to start was ignored due
     * to another script already executing.
     */
    bool run(std::shared_ptr<MotionScriptRunner> const& scriptRunner);

    bool isRunning();

  private:
    MotionScriptModule(const MotionScriptModule&) = delete;
    MotionScriptModule& operator=(const MotionScriptModule&) = delete;

    std::shared_ptr<MotionScriptRunner> d_runner;
  };
}
