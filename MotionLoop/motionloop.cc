#include "motionloop.hh"

#include "../AgentState/agentstate.hh"
#include "../BodyControl/bodycontrol.hh"
#include "../CM730/cm730.hh"
#include "../CM730Platform/CM730Linux/cm730linux.hh"
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
#include "../ThreadUtil/threadutil.hh"
#include "../util/ccolor.hh"

#include <time.h>
#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

MotionLoop::MotionLoop(shared_ptr<DebugControl> debugControl)
: d_debugControl(debugControl),
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

  if (d_isStarted)
  {
    log::error("MotionLoop::start") << "Cannot start motion loop as it is already running";
    return false;
  }

  // Connect to hardware subcontroller
  auto cm730DevicePath = Config::getStaticValue<string>("hardware.cm730-path");
  log::info("MotionLoop::start") << "Using CM730 Device Path: " << cm730DevicePath;
  auto cm730Linux = unique_ptr<CM730Linux>(new CM730Linux(cm730DevicePath));
  d_cm730 = unique_ptr<CM730>(new CM730(move(cm730Linux)));
  d_haveBody = d_cm730->connect();

  if (!d_haveBody)
  {
    log::error("MotionLoop::start") << "Failed to connect to CM730";
  }
  else
  {
    d_cm730->torqueEnable(true);
  }

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
  if (error == EPERM)
  {
    log::error("MotionLoop::start") << "Not permitted to start the motion thread. Did you use sudo?";
    return false;
  }
  else if (error != 0)
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

  if (d_haveBody)
    d_cm730->torqueEnable(false);

  d_isStopRequested = false;
  d_isStarted = false;
}

void *MotionLoop::threadMethod(void *param)
{
  log::info("MotionLoop::threadMethod") << "Started";

  ThreadUtil::setThreadId(ThreadId::MotionLoop);

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
    AgentState::set(make_shared<MotionTimingState const>(t.flush(), loop->d_cycleNumber));
  }

  log::verbose("MotionLoop::threadMethod") << "Exiting";

  pthread_exit(NULL);
}

bool MotionLoop::applyJointMotionTasks(SequentialTimer& t)
{
  auto tasks = AgentState::get<MotionTaskState>();

  if (!tasks || tasks->isEmpty())
    return false;

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

    t.timeEvent(module->getName());
  }

  return true;
}

bool MotionLoop::writeJointData(SequentialTimer& t)
{
  //
  // WRITE UPDATE
  //

  uchar dirtyDeviceCount = 0;
  Range<int> addrRange;
  for (auto const& joint : d_bodyControl->getJoints())
  {
    if (joint->isDirty())
    {
      dirtyDeviceCount++;
      addrRange.expand(joint->getModifiedAddressRange());
    }
  }

  if (dirtyDeviceCount != 0)
  {
    // Prepare the parameters of a SyncWrite instruction
    int bytesPerDevice = 1 + addrRange.size() + 1;

    uchar parameters[dirtyDeviceCount * bytesPerDevice];
    int n = 0;
    for (auto const& joint : d_bodyControl->getJoints())
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

  return dirtyDeviceCount != 0;
}

void MotionLoop::step(SequentialTimer& t)
{
  d_cycleNumber++;

  // Don't write until we've read
  if (d_readYet)
  {
    //
    // LET MOTION MODULES UPDATE BODY CONTROL
    //

    t.enter("Apply Motion Module");
    if (applyJointMotionTasks(t))
    {
      // Joints were modified. Write to MX28s.
      bool anythingChanged = d_haveBody
        ? writeJointData(t)
        : true;

      //
      // Update BodyControlState
      //
      if (anythingChanged)
      {
        // TODO only create if someone is listening to this in the debugger
        AgentState::set(make_shared<BodyControlState const>(d_bodyControl, d_cycleNumber));
        t.timeEvent("Set BodyControlState");
      }
    }
    t.exit();

    if (d_debugControl->isDirty())
    {
      if (d_haveBody)
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
      else
      {
        d_debugControl->clearDirtyFlags();
        t.timeEvent("Write to CM730 (Dummy)");
      }
    }
  }

  auto hw = d_haveBody
    ? readHardwareState(t)
    : readHardwareStateFake(t);

  // Ensure we were able to read something
  if (hw == nullptr)
    return;

  AgentState::set(hw);

  AgentState::set(make_shared<BodyState const>(hw, d_cycleNumber));
  t.timeEvent("Update BodyState");

  if (!d_readYet)
  {
    d_bodyControl->updateFromHardwareState(hw);
    d_readYet = true;
  }

  t.enter("Observers");
  AgentState::callbackObservers(ThreadId::MotionLoop, t);
  t.exit();
}

shared_ptr<HardwareState const> MotionLoop::readHardwareState(SequentialTimer& t)
{
  assert(d_haveBody);

  //
  // READ DATA
  //

  if (d_staticHardwareStateUpdateNeeded)
  {
    if (updateStaticHardwareState())
      d_staticHardwareStateUpdateNeeded = false;
    t.timeEvent("Read StaticHardwareState");
  }

  CommResult res = d_cm730->bulkRead(d_dynamicBulkRead.get());
  t.timeEvent("Read from CM730");

  if (res != CommResult::SUCCESS)
  {
    // TODO if this occurs N times in a row, consider recreating the CM730 instance (perhaps someone pressed the hardware reset button)
    log::warning("MotionLoop::process") << "Bulk read failed (" << CM730::getCommResultName(res) << ") -- skipping update of HardwareState";
    return nullptr;
  }

  auto cm730Snapshot = unique_ptr<CM730Snapshot const>(new CM730Snapshot(d_dynamicBulkRead->getBulkReadData(CM730::ID_CM)));

  auto mx28Snapshots = vector<unique_ptr<MX28Snapshot const>>();
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    mx28Snapshots.push_back(unique_ptr<MX28Snapshot const>(new MX28Snapshot(jointId, d_dynamicBulkRead->getBulkReadData(jointId))));

  //
  // UPDATE HARDWARE STATE
  //

  auto rxBytes = d_cm730->getReceivedByteCount();
  auto txBytes = d_cm730->getTransmittedByteCount();

  auto hw = make_shared<HardwareState const>(move(cm730Snapshot), move(mx28Snapshots), rxBytes, txBytes, d_cycleNumber);
  t.timeEvent("Update HardwareState");
  return hw;
}

bool MotionLoop::updateStaticHardwareState()
{
  assert(d_haveBody);

  CommResult res = d_cm730->bulkRead(d_staticBulkRead.get());

  if (res != CommResult::SUCCESS)
  {
    log::warning("MotionLoop::updateStaticHardwareState") << "Bulk read failed -- skipping update of StaticHardwareState";
    return false;
  }

  auto cm730State = make_shared<StaticCM730State>(d_staticBulkRead->getBulkReadData(CM730::ID_CM));

  auto mx28States = vector<shared_ptr<StaticMX28State const>>();
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    mx28States.push_back(make_shared<StaticMX28State>(jointId, d_staticBulkRead->getBulkReadData(jointId)));

  AgentState::set(make_shared<StaticHardwareState const>(cm730State, mx28States));
  return true;
}

shared_ptr<HardwareState const> MotionLoop::readHardwareStateFake(SequentialTimer& t)
{
  assert(!d_haveBody);

  //
  // Dummy StaticHardwareState
  //

  if (d_staticHardwareStateUpdateNeeded)
  {
    auto mx28States = vector<shared_ptr<StaticMX28State const>>();
    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
      auto mx28State = make_shared<StaticMX28State>(jointId);
      mx28State->alarmLed = MX28Alarm();
      mx28State->alarmShutdown = MX28Alarm();
      mx28State->torqueEnable = true;
      mx28States.push_back(mx28State);
    }

    auto cm730State = make_shared<StaticCM730State>();
    AgentState::set(make_shared<StaticHardwareState const>(cm730State, mx28States));

    d_staticHardwareStateUpdateNeeded = false;
    t.timeEvent("Read StaticHardwareState");
  }

  //
  // Dummy HardwareState
  //

  auto cm730State = new CM730Snapshot();
  cm730State->acc = {};
  cm730State->accRaw = Eigen::Vector3i(512, 512, 512);
  cm730State->eyeColor = d_debugControl->getEyeColour().toRgbUnitVector();
  cm730State->foreheadColor = d_debugControl->getForeheadColour().toRgbUnitVector();
  cm730State->gyro = {};
  cm730State->gyroRaw = Eigen::Vector3i(512, 512, 512);
  cm730State->isLed2On = d_debugControl->isRedPanelLedLit();
  cm730State->isLed3On = d_debugControl->isBluePanelLedLit();
  cm730State->isLed4On = d_debugControl->isGreenPanelLedLit();
  cm730State->isModeButtonPressed = false;
  cm730State->isPowered = true;
  cm730State->isStartButtonPressed = false;
  cm730State->voltage = 12.3f;

  vector<unique_ptr<MX28Snapshot const>> mx28States;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    auto jointControl = d_bodyControl->getJoint((JointId)jointId);
    double rads = jointControl->getRadians();

    auto mx28State = new MX28Snapshot(jointId);
    mx28State->presentLoad = 0;
    mx28State->presentPosition = rads;
    mx28State->presentPositionValue = MX28::rads2Value(rads);
    mx28State->presentSpeedRPM = 0;
    mx28State->presentTemp = 40;
    mx28State->presentVoltage = 12;
    mx28States.push_back(unique_ptr<MX28Snapshot const>(mx28State));
  }

  auto hw = make_shared<HardwareState const>(unique_ptr<CM730Snapshot const>(cm730State), move(mx28States), 0, 0, d_cycleNumber);
  t.timeEvent("Update HardwareState (Dummy)");
  return hw;
}
