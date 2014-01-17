#include "motionloop.hh"

#include "../AgentState/agentstate.hh"
#include "../BodyControl/bodycontrol.hh"
#include "../CM730/cm730.hh"
#include "../CM730Snapshot/cm730snapshot.hh"
#include "../Config/config.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../SequentialTimer/sequentialtimer.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/BodyControlState/bodycontrolstate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/MotionTaskState/motiontaskstate.hh"
#include "../StateObject/StaticHardwareState/statichardwarestate.hh"
#include "../StateObject/TimingState/timingstate.hh"
#include "../ThreadId/threadid.hh"
#include "../util/ccolor.hh"

#include <time.h>
#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

MotionLoop::MotionLoop(unique_ptr<CM730> cm730, shared_ptr<DebugControl> debugControl)
: d_cm730(move(cm730)),
  d_debugControl(debugControl),
  d_isStarted(false),
  d_isStopRequested(false),
  d_loopDurationMillis(8),
  d_readYet(false),
  d_cycleNumber(0),
  d_staticHardwareStateUpdateNeeded(true)
{
  d_bodyControl = make_shared<BodyControl>();

  d_dynamicBulkRead = unique_ptr<BulkRead>(
    new BulkRead(CM730::P_DXL_POWER, CM730::P_VOLTAGE,
                 MX28::P_PRESENT_POSITION_L, MX28::P_PRESENT_TEMPERATURE));

  d_staticBulkRead = unique_ptr<BulkRead>(
    new BulkRead(CM730::P_MODEL_NUMBER_L, CM730::P_RETURN_LEVEL,
                 MX28::P_MODEL_NUMBER_L, MX28::P_LOCK));

  // add an action that sets d_staticHardwareStateUpdateNeeded true
  Config::addAction("motion-loop.query-static-hardware-state", "Query static HW state",
    [this]() { d_staticHardwareStateUpdateNeeded = true; }
  );

  for (uchar i = 0; i < (uchar)JointId::MAX; i++)
    d_offsets[i] = 0;
}

MotionLoop::~MotionLoop()
{
  if (d_isStarted && !d_isStopRequested)
    stop();
}

void MotionLoop::addModule(shared_ptr<MotionModule> module)
{
  assert(module);

  // Initialise modules each time they are added
  module->initialize();

  d_modules.push_back(module);
}

void MotionLoop::removeModule(shared_ptr<MotionModule> module)
{
  d_modules.remove(module);
}

bool MotionLoop::start()
{
  log::info("MotionLoop::start") << "Starting";

  d_cm730->torqueEnable(true);

  d_readYet = false;

  // Initialise default thread attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  // Set the scheduling policy as 'RR'
  int error = pthread_attr_setschedpolicy(&attr, SCHED_RR);
  if (error != 0)
  {
    log::error("MotionLoop::start") << "Error setting thread scheduling policy as RR: " << error;
    return false;
  }

  // Set the scheduler inheritence (no inheritance)
  error = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  if (error != 0)
  {
    log::error("MotionLoop::start") << "Error setting thread scheduler inheritence as explicit: " << error;
    return false;
  }

  // Set the thread as having real-time priority (requires elevated permissions)
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = 31;
  error = pthread_attr_setschedparam(&attr, &param);
  if (error != 0)
  {
    log::error("MotionLoop::start") << "Error setting thread priority as realtime: " << error;
    return false;
  }

  // Create and start the thread
  error = pthread_create(&d_thread, &attr, threadMethod, this);
  if (error != 0)
  {
    log::error("MotionLoop::start") << "Error starting thread: " << error;
    return false;
  }

  log::info("MotionLoop::start") << "Started";

  d_isStarted = true;
  return true;
}

void MotionLoop::stop()
{
  if (!d_isStarted || d_isStopRequested)
    return;

  log::verbose("MotionLoop::stop") << "Stopping";

  // set the flag to end the thread
  d_isStopRequested = true;

  // wait for the thread to end
  int error;
  if ((error = pthread_join(d_thread, NULL)) != 0)
    exit(-1);

  log::info("MotionLoop::stop") << "Stopped";

  d_cm730->torqueEnable(false);

  d_isStopRequested = false;
  d_isStarted = false;
}

void *MotionLoop::threadMethod(void *param)
{
  log::info("MotionLoop::threadMethod") << "Started";

  ThreadId::setThreadId(ThreadIds::MotionLoop);

  MotionLoop *loop = static_cast<MotionLoop*>(param);
  static struct timespec next_time;
  clock_gettime(CLOCK_MONOTONIC, &next_time);

  while (!loop->d_isStopRequested)
  {
    // TODO this will always increment by <8ms, even if something stalled
    next_time.tv_sec += (next_time.tv_nsec + loop->d_loopDurationMillis * 1000000) / 1000000000;
    next_time.tv_nsec = (next_time.tv_nsec + loop->d_loopDurationMillis * 1000000) % 1000000000;

    SequentialTimer t;

    loop->step(t);

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
    t.timeEvent("Sleep");

    // Set timing data for the motion cycle
    AgentState::getInstance().set(make_shared<MotionTimingState const>(t.flush(), loop->d_cycleNumber));
  }

  log::verbose("MotionLoop::threadMethod") << "Exiting";

  pthread_exit(NULL);
}

void MotionLoop::step(SequentialTimer& t)
{
  d_cycleNumber++;

  if (d_readYet)
  {
    //
    // LET MOTION MODULES UPDATE BODY CONTROL
    //

    auto tasks = AgentState::get<MotionTaskState>();

    if (tasks && !tasks->isEmpty())
    {
      for (pair<MotionModule*, shared_ptr<JointSelection>> const& pair : *tasks->getModuleJointSelection())
      {
        MotionModule* module = pair.first;
        shared_ptr<JointSelection> jointSelection = pair.second;

        assert(module);
        assert(jointSelection);

        module->step(jointSelection);

        if (jointSelection->hasHead())
          module->applyHead(d_bodyControl->getHeadSection());

        if (jointSelection->hasArms())
          module->applyArms(d_bodyControl->getArmSection());

        if (jointSelection->hasLegs())
          module->applyLegs(d_bodyControl->getLegSection());

        t.timeEvent("Step & Apply Module (" + module->getName() + ")");
      }

      //
      // WRITE UPDATE
      //

      int dirtyDeviceCount = 0;
      Range<int> addrRange;
      for (shared_ptr<JointControl> joint : d_bodyControl->getJoints())
      {
        if (joint->isDirty())
        {
          dirtyDeviceCount++;
          addrRange.expand(joint->getModifiedAddressRange());
        }
      }

      if (dirtyDeviceCount > 0)
      {
        // Prepare the parameters of a SyncWrite instruction
        int bytesPerDevice = 1 + addrRange.size() + 1;

        uchar parameters[dirtyDeviceCount * bytesPerDevice];
        int n = 0;
        for (shared_ptr<JointControl> joint : d_bodyControl->getJoints())
        {
          if (joint->isDirty())
          {
            // Specify the goal position, and apply any calibration offset
            int goalPosition = joint->getValue() + d_offsets[joint->getId()];

            parameters[n++] = joint->getId();

            if (addrRange.contains(MX28::P_D_GAIN))   parameters[n++] = joint->getDGain();
            if (addrRange.contains(MX28::P_I_GAIN))   parameters[n++] = joint->getIGain();
            if (addrRange.contains(MX28::P_P_GAIN))   parameters[n++] = joint->getPGain();
            if (addrRange.contains(MX28::P_RESERVED)) parameters[n++] = 0;

            if (addrRange.contains(MX28::P_GOAL_POSITION_L))
            {
              assert(addrRange.contains(MX28::P_GOAL_POSITION_H));
              parameters[n++] = CM730::getLowByte(goalPosition);
              parameters[n++] = CM730::getHighByte(goalPosition);
            }

            joint->clearDirty();
          }
        }
        assert(n == dirtyDeviceCount * bytesPerDevice);

        //
        // Send the SyncWrite message, if anything changed
        //

        d_cm730->syncWrite(addrRange.min(), bytesPerDevice, dirtyDeviceCount, parameters);
      }

      t.timeEvent("Write to MX28s");

      //
      // Update BodyControlState
      //
      // TODO only create if someone is listening to this in the debugger
      if (dirtyDeviceCount > 0)
      {
        AgentState::getInstance().set(make_shared<BodyControlState const>(d_bodyControl, d_cycleNumber));

        t.timeEvent("Set BodyControlState");
      }
    }

    if (d_debugControl->isDirty())
    {
      int bytesToWrite = 1 + (1 + 2 + 2);
      uchar parameters[bytesToWrite];
      int n = 0;

      ushort forehead = d_debugControl->getForeheadColourShort();
      ushort eye = d_debugControl->getEyeColourShort();

      parameters[n++] = CM730::ID_CM;
      parameters[n++] = d_debugControl->getPanelLedByte();
      parameters[n++] = CM730::getLowByte(forehead);
      parameters[n++] = CM730::getHighByte(forehead);
      parameters[n++] = CM730::getLowByte(eye);
      parameters[n++] = CM730::getHighByte(eye);

      d_cm730->syncWrite(CM730::P_LED_PANEL, bytesToWrite, 1, parameters);

      d_debugControl->clearDirtyFlags();
      t.timeEvent("Write to CM730");
    }
  }

  //
  // READ DATA
  //

  if (d_staticHardwareStateUpdateNeeded)
  {
    updateStaticHardwareState();
    d_staticHardwareStateUpdateNeeded = false;
    t.timeEvent("Read StaticHardwareState");
  }

  CommResult res = d_cm730->bulkRead(d_dynamicBulkRead.get());
  t.timeEvent("Read from CM730");

  if (res != CommResult::SUCCESS)
  {
    // TODO if this occurs N times in a row, consider recreating the CM730 instance (perhaps someone pressed the hardware reset button)
    log::warning("MotionLoop::process") << "Bulk read failed (" << CM730::getCommResultName(res) << ") -- skipping update of HardwareState";
    return;
  }

  auto cm730Snapshot = make_shared<CM730Snapshot>(d_dynamicBulkRead->getBulkReadData(CM730::ID_CM));

  auto mx28Snapshots = vector<shared_ptr<MX28Snapshot const>>();
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    mx28Snapshots.push_back(make_shared<MX28Snapshot>(d_dynamicBulkRead->getBulkReadData(jointId), jointId));

  //
  // UPDATE HARDWARE STATE
  //

  auto rxBytes = d_cm730->getReceivedByteCount();
  auto txBytes = d_cm730->getTransmittedByteCount();

  AgentState::getInstance().set(make_shared<HardwareState const>(cm730Snapshot, mx28Snapshots, rxBytes, txBytes, d_cycleNumber));
  t.timeEvent("Update HardwareState");

  if (!d_readYet)
  {
    d_bodyControl->updateFromHardwareState();
    d_readYet = true;
  }
}

void MotionLoop::updateStaticHardwareState()
{
  CommResult res = d_cm730->bulkRead(d_staticBulkRead.get());

  if (res != CommResult::SUCCESS)
  {
    log::warning("MotionLoop::updateStaticHardwareState") << "Bulk read failed -- skipping update of StaticHardwareState";
    return;
  }

  auto cm730State = make_shared<StaticCM730State>(d_staticBulkRead->getBulkReadData(CM730::ID_CM));

  auto mx28States = vector<shared_ptr<StaticMX28State const>>();
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    mx28States.push_back(make_shared<StaticMX28State>(d_staticBulkRead->getBulkReadData(jointId), jointId));

  AgentState::getInstance().set(make_shared<StaticHardwareState const>(cm730State, mx28States));
}
