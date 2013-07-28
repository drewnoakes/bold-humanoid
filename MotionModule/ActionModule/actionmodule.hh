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
  class BodySection;

  class ActionModule : public MotionModule
  {
  private:
    std::shared_ptr<MotionScriptFile> d_file;
    std::shared_ptr<MotionScriptPage> d_playingPage;
    std::shared_ptr<MotionScriptPage> m_NextPlayPage;
    uchar m_CurrentStep;

    int d_playingPageIndex;
    /// Whether the next step will be the first of the action
    bool d_isFirstStepOfAction;
    int m_PageStepCount;
    bool d_isRunning;
    bool d_isStopRequested;
    bool d_playingFinished;

    bool d_active[21];
    uchar d_pGains[21];
    ushort d_values[21];

    std::vector<std::shared_ptr<Control const>> d_controls;

    bool isJointActive(uchar jointId) const { return d_active[jointId]; }

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

    bool start(int pageIndex);

    /** Play the first page having the specified name. */
    bool start(std::string const& pageName);

    /// Request the action to come to a stop soon.
    void stop();

    /// Stop immediately, irrespective of the current playing position.
    void brake();

    bool isRunning();
  };
}
