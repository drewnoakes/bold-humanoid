#pragma once

#include <pthread.h>
#include <memory>
#include <list>

#include "../MotionModule/motionmodule.hh"

namespace bold
{
  class BodyControl;
  class BulkRead;
  class CM730;
  class DebugControl;
  class SequentialTimer;

  class MotionLoop
  {
  public:
    MotionLoop(std::unique_ptr<CM730> cm730, std::shared_ptr<DebugControl> debugControl);

    ~MotionLoop();

    bool start();
    void stop();

    void addModule(std::shared_ptr<MotionModule> module);
    void removeModule(std::shared_ptr<MotionModule> module);

    /// TODO allow specifying these in config
    /// Set of static offsets to be added to all target positions sent to hardware.
    /// May be used to compensate for angular positional errors.
    /// See also the offset_tuner project.
    int d_offsets[(uchar)JointId::MAX + 1];

  private:
    void step(SequentialTimer& t);

    void updateStaticHardwareState();

    std::list<std::shared_ptr<MotionModule>> d_modules;
    std::unique_ptr<CM730> d_cm730;
    std::shared_ptr<DebugControl> d_debugControl;
    std::shared_ptr<BodyControl> d_bodyControl;
    std::unique_ptr<BulkRead> d_dynamicBulkRead;
    std::unique_ptr<BulkRead> d_staticBulkRead;

    /// When false, calls to process have no effect
    bool d_isStarted;
    bool d_isStopRequested;
    unsigned d_loopDurationMillis;

    pthread_t d_thread;

    /// Whether the loop has read any values yet.
    bool d_readYet;

    ulong d_cycleNumber;

    bool d_staticHardwareStateUpdateNeeded;

    /// The method that governs the thread's lifetime and operation
    static void *threadMethod(void *param);
  };
}
