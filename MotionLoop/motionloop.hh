#pragma once

#include <pthread.h>
#include <memory>
#include <list>
#include <sigc++/signal.h>

#include "../CM730CommsModule/cm730commsmodule.hh"
#include "../MotionModule/motionmodule.hh"
#include "../util/loop.hh"

namespace bold
{
  class BodyControl;
  class BodyModel;
  class BulkRead;
  class CM730;
  class DebugControl;
  class HardwareState;
  class SequentialTimer;
  class Voice;

  class MotionLoop : public Loop
  {
  public:
    MotionLoop(std::shared_ptr<DebugControl> debugControl);

    void addMotionModule(std::shared_ptr<MotionModule> const& module);
    void addCommsModule(std::shared_ptr<CM730CommsModule> const& module);

    /// Signal raised whenever the motion loop fails to read from the CM730.
    /// Callback is passed the number of consecutive failures.
    sigc::signal<void, uint> onReadFailure;

  private:
    virtual void onLoopStart() override;
    virtual void onStep(ulong cycleNumber) override;
    virtual void onStopped() override;

    void initialiseHardwareTables();

    bool applyJointMotionTasks(SequentialTimer& t);
    bool writeJointData(SequentialTimer& t);
    std::shared_ptr<HardwareState const> readHardwareState(SequentialTimer& t);
    std::shared_ptr<HardwareState const> readHardwareStateFake(SequentialTimer& t);

    bool updateStaticHardwareState();

    std::unique_ptr<CM730> d_cm730;
    std::shared_ptr<DebugControl> d_debugControl;
    std::shared_ptr<BodyModel> d_bodyModel;
    std::shared_ptr<BodyControl> d_bodyControl;
    std::unique_ptr<BulkRead> d_dynamicBulkRead;
    std::unique_ptr<BulkRead> d_staticBulkRead;

    std::vector<std::shared_ptr<MotionModule>> d_motionModules;
    std::vector<std::shared_ptr<CM730CommsModule>> d_commsModules;

    LoopRegulator d_loopRegulator;

    /// Set of static offsets to be added to all target positions sent to hardware.
    /// May be used to compensate for angular positional errors.
    /// See also the offset_tuner project.
    int d_offsets[(uchar)JointId::MAX + 1];

    /// Whether we have connected to a CM730 subcontroller.
    bool d_haveBody;
    /// Whether the CM730 subcontroller is powered. Updated every N cycles.
    bool d_isCM730PowerEnabled;
    /// Whether the loop has read any values yet.
    bool d_readYet;

    bool d_staticHardwareStateUpdateNeeded;
    uint d_consecutiveReadFailureCount;

    bool d_powerChangeNeeded;
    bool d_powerChangeToValue;
    bool d_torqueChangeNeeded;
    bool d_torqueChangeToValue;
  };
}
