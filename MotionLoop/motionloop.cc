#include "motionloop.hh"

#include "../BodyControl/bodycontrol.hh"
#include "../CM730/cm730.hh"
#include "../CM730Platform/CM730Linux/cm730linux.hh"
#include "../CM730Snapshot/cm730snapshot.hh"
#include "../Config/config.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../SequentialTimer/sequentialtimer.hh"
#include "../State/state.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/BodyControlState/bodycontrolstate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/MotionTaskState/motiontaskstate.hh"
#include "../StateObject/StaticHardwareState/statichardwarestate.hh"
#include "../StateObject/TimingState/timingstate.hh"
#include "../ThreadUtil/threadutil.hh"
#include "../util/ccolor.hh"
#include "../util/memory.hh"

#include <time.h>
#include <string.h>

using namespace bold;
using namespace std;

MotionLoop::MotionLoop(shared_ptr<DebugControl> debugControl)
: d_debugControl(debugControl),
  d_haveBody(false),
  d_isStarted(false),
  d_isStopRequested(false),
  d_loopDurationMillis(8),
  d_readYet(false),
  d_cycleNumber(0),
  d_staticHardwareStateUpdateNeeded(true)
{
  d_bodyControl = make_shared<BodyControl>();

  d_dynamicBulkRead = make_unique<BulkRead>(
    CM730::P_DXL_POWER, CM730::P_VOLTAGE,
    MX28::P_PRESENT_POSITION_L, MX28::P_PRESENT_TEMPERATURE);

  d_staticBulkRead = make_unique<BulkRead>(
    CM730::P_MODEL_NUMBER_L, CM730::P_RETURN_LEVEL,
    MX28::P_MODEL_NUMBER_L, MX28::P_LOCK);

  Config::addAction("hardware.query-static-hardware-state", "Query static HW state", [this]() { d_staticHardwareStateUpdateNeeded = true; });
//   Config::addAction("hardware.cm730-power-on",  "CM730 On",  [this]() { d_powerChangeToValue = true;  d_powerChangeNeeded = true; });
//   Config::addAction("hardware.cm730-power-off", "CM730 Off", [this]() { d_powerChangeToValue = false; d_powerChangeNeeded = true; });
  Config::addAction("hardware.motor-torque-on",  "Torque On",  [this]() { d_torqueChangeToValue = true;  d_torqueChangeNeeded = true; });
  Config::addAction("hardware.motor-torque-off", "Torque Off", [this]() { d_torqueChangeToValue = false; d_torqueChangeNeeded = true; });

  d_offsets[0] = 0;
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    stringstream path;
    path << "hardware.offsets.joint-" << (int)jointId;
    Config::getSetting<int>(path.str())->track([this, jointId ](int value)
    {
      d_offsets[jointId] = value;
      d_bodyControl->getJoint((JointId)jointId)->notifyOffsetChanged();
    });
  }
}

MotionLoop::~MotionLoop()
{
  if (d_isStarted && !d_isStopRequested)
    stop();
}

void MotionLoop::addModule(shared_ptr<MotionModule> const& module)
{
  ASSERT(module);
  d_modules.push_back(module);
}

void MotionLoop::removeModule(shared_ptr<MotionModule>const& module)
{
  ASSERT(module);
  d_modules.remove(module);
}

bool MotionLoop::start()
{
  log::verbose("MotionLoop::start") << "Starting";

  if (d_isStarted)
  {
    log::error("MotionLoop::start") << "Cannot start motion loop as it is already running";
    return false;
  }

  // Connect to hardware subcontroller
  auto cm730DevicePath = Config::getStaticValue<string>("hardware.cm730-path");
  log::info("MotionLoop::start") << "Using CM730 Device Path: " << cm730DevicePath;
  d_cm730 = unique_ptr<CM730>(new CM730(make_unique<CM730Linux>(cm730DevicePath)));

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

  // Set the scheduler inheritance (no inheritance)
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

  d_isStopRequested = false;
  d_isStarted = false;
}

void *MotionLoop::threadMethod(void *param)
{
  log::info("MotionLoop::threadMethod") << "Started";

  ThreadUtil::setThreadId(ThreadId::MotionLoop);

  MotionLoop *loop = static_cast<MotionLoop*>(param);

  loop->d_haveBody = loop->d_cm730->connect();

  if (!loop->d_haveBody)
  {
    log::warning("MotionLoop::threadMethod") << "Motion loop running without a body";
  }
  else
  {
    // Set all CM730/MX28 values to a known set of initial values before continuing
    loop->initialiseHardwareTables();

    if (!loop->d_cm730->torqueEnable(true))
      log::error("MotionLoop::threadMethod") << "Error enabling torque";
  }

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
    State::make<MotionTimingState>(t.flush(), loop->d_cycleNumber);
  }

  if (loop->d_haveBody)
  {
    if (!loop->d_cm730->torqueEnable(false))
      log::error("MotionLoop::threadMethod") << "Error disabling torque";
  }

  // Destroy the CM730 object on the motion thread
  loop->d_cm730.reset();

  log::verbose("MotionLoop::threadMethod") << "Exiting";

  pthread_exit(NULL);
}

void MotionLoop::initialiseHardwareTables()
{
  ASSERT(d_haveBody);

  log::verbose("MotionLoop::initialiseHardwareTables") << "Starting";

  //
  // Helper functions
  //

  const int retryCount = 10;

  auto writeByteWithRetry = [this](uchar jointId, uchar address, uchar value)
  {
    int failCount = 0;
    while (true)
    {
      MX28Alarm error;
      CommResult res = d_cm730->writeByte(jointId, address, value, &error);

      if (res == CommResult::SUCCESS)
      {
        if (error.hasError())
          log::error("MotionLoop::initialiseHardwareTables") << "Error reported by " << JointName::getEnumName(jointId) << " when writing value " << value << " to " << MX28::getAddressName(address) << ": " << error;
        return;
      }
      else
      {
        failCount++;
        if (failCount == retryCount)
        {
          log::error("MotionLoop::initialiseHardwareTables") << "Communication problem writing " << MX28::getAddressName(address) << " to " << JointName::getEnumName(jointId) << ") after " << retryCount << " retries: " << CM730::getCommResultName(res);
          return;
        }
        usleep(50000); // 50ms
      }
    }
  };

  auto writeWordWithRetry = [this](uchar jointId, uchar address, ushort value)
  {
    int failCount = 0;
    while (true)
    {
      MX28Alarm error;
      CommResult res = d_cm730->writeWord(jointId, address, value, &error);

      if (res == CommResult::SUCCESS)
      {
        if (error.hasError())
          log::error("MotionLoop::initialiseHardwareTables") << "Error reported by " << JointName::getEnumName(jointId) << " when writing value " << value << " to " << MX28::getAddressName(address) << ": " << error;
        return;
      }
      else
      {
        failCount++;
        if (failCount == retryCount)
        {
          log::error("MotionLoop::initialiseHardwareTables") << "Communication problem writing " << MX28::getAddressName(address) << " to " << JointName::getEnumName(jointId) << ") after " << retryCount << " retries: " << CM730::getCommResultName(res);
          return;
        }
        usleep(50000); // 50ms
      }
    }
  };

  auto ping = [this](uchar jointId) -> bool
  {
    MX28Alarm error;
    CommResult res = d_cm730->ping(jointId, &error);

    if (res != CommResult::SUCCESS)
    {
      log::error("MotionLoop::initialiseHardwareTables") << "Communication problem pinging " << JointName::getEnumName(jointId) << ": " << CM730::getCommResultName(res);
      return false;
    }

    if (error.hasError())
    {
      log::error("MotionLoop::initialiseHardwareTables") << "Error when pinging " << JointName::getEnumName(jointId) << ": " << error;
      return false;
    }

    return true;
  };

  //
  // Prepare values to be written
  //

  // Under which circumstances do we want the LED to light up?
  MX28Alarm alarmLed;
  alarmLed.setOverheatedFlag();
  alarmLed.setOverloadFlag();

  // Under which circumstances do we want the MX28 to automatically shut down?
  MX28Alarm alarmShutdown;
  alarmShutdown.setOverheatedFlag();
  alarmShutdown.setOverloadFlag();

  int tempLimit = Config::getStaticValue<int>("hardware.limits.temperature-centigrade");
  Range<double> voltageRange = Config::getStaticValue<Range<double>>("hardware.limits.voltage-range");

  //
  // Loop through all MX28s
  //

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    if (!ping(jointId))
      continue;

    stringstream path;
    path << "hardware.limits.angle-limits.joint-" << (int)jointId;
    Range<double> rangeDegs = Config::getStaticValue<Range<double>>(path.str());

    writeByteWithRetry(jointId, MX28::P_RETURN_DELAY_TIME, 0);
    writeByteWithRetry(jointId, MX28::P_RETURN_LEVEL, 2);
    writeWordWithRetry(jointId, MX28::P_CW_ANGLE_LIMIT_L, MX28::degs2Value(rangeDegs.min()));
    writeWordWithRetry(jointId, MX28::P_CCW_ANGLE_LIMIT_L, MX28::degs2Value(rangeDegs.max()));
    writeByteWithRetry(jointId, MX28::P_HIGH_LIMIT_TEMPERATURE, MX28::centigrade2Value(tempLimit));
    writeByteWithRetry(jointId, MX28::P_LOW_LIMIT_VOLTAGE, MX28::voltage2Value(voltageRange.min()));
    writeByteWithRetry(jointId, MX28::P_HIGH_LIMIT_VOLTAGE, MX28::voltage2Value(voltageRange.max()));
    writeWordWithRetry(jointId, MX28::P_MAX_TORQUE_L, MX28::MAX_TORQUE);
    writeByteWithRetry(jointId, MX28::P_ALARM_LED, alarmLed.getFlags());
    writeByteWithRetry(jointId, MX28::P_ALARM_SHUTDOWN, alarmShutdown.getFlags());
  }

  log::info("MotionLoop::initialiseHardwareTables") << "All MX28 data tables initialised";
}

bool MotionLoop::applyJointMotionTasks(SequentialTimer& t)
{
  auto tasks = State::get<MotionTaskState>();

  if (!tasks || tasks->isEmpty())
    return false;

  for (pair<MotionModule*, shared_ptr<JointSelection>> const& pair : *tasks->getModuleJointSelection())
  {
    MotionModule* module = pair.first;
    shared_ptr<JointSelection> jointSelection = pair.second;

    ASSERT(module);
    ASSERT(jointSelection);

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
        parameters[n++] = joint->getId();

        if (addrRange.contains(MX28::P_D_GAIN))   parameters[n++] = joint->getDGain();
        if (addrRange.contains(MX28::P_I_GAIN))   parameters[n++] = joint->getIGain();
        if (addrRange.contains(MX28::P_P_GAIN))   parameters[n++] = joint->getPGain();
        if (addrRange.contains(MX28::P_RESERVED)) parameters[n++] = 0;

        if (addrRange.contains(MX28::P_GOAL_POSITION_L))
        {
          // Specify the goal position, and apply any calibration offset
          int goalPosition = joint->getValue() + d_offsets[joint->getId()];
          ushort value = static_cast<ushort>(Math::clamp(goalPosition, 0, (int)MX28::MAX_VALUE));

          ASSERT(addrRange.contains(MX28::P_GOAL_POSITION_H));
          parameters[n++] = CM730::getLowByte(value);
          parameters[n++] = CM730::getHighByte(value);
        }

        joint->clearDirty();
      }
    }
    ASSERT(n == dirtyDeviceCount * bytesPerDevice);

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
  // Rate the limit at which we make this call to the CM730.
  if (d_cycleNumber % 60 == 0 && d_haveBody)
  {
    d_isCM730PowerEnabled = d_cm730->isPowerEnabled();
    t.timeEvent("Check Power");
  }

  // When unpowered, don't perform any update at all in the cycle
  if (d_haveBody && !d_isCM730PowerEnabled)
    return;

  d_cycleNumber++;

  // Don't write until we've read
  if (d_readYet)
  {
    if (d_haveBody)
    {
      if (d_powerChangeNeeded)
      {
        if (!d_cm730->powerEnable(d_powerChangeToValue))
          log::error("MotionLoop::step") << "Error setting power to " << d_powerChangeToValue;
        d_powerChangeNeeded = false;
      }

      if (d_torqueChangeNeeded)
      {
        if (!d_cm730->torqueEnable(d_torqueChangeToValue))
          log::error("MotionLoop::step") << "Error setting torque to " << d_torqueChangeToValue;
        d_torqueChangeNeeded = false;
      }
    }

    //
    // LET MOTION MODULES UPDATE BODY CONTROL
    //

    t.enter("Apply Motion Module");
    applyJointMotionTasks(t);
    t.exit();

    //
    // WRITE TO MX28S
    //

    bool anythingChanged = d_haveBody
      ? writeJointData(t)
      : true;

    //
    // Update BodyControlState
    //

    if (anythingChanged)
    {
      State::make<BodyControlState>(d_bodyControl, d_cycleNumber);
      t.timeEvent("Set BodyControlState");
    }

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

  State::set(hw);

  State::make<BodyState>(hw, d_bodyControl, d_cycleNumber);
  t.timeEvent("Update BodyState");

  if (!d_readYet)
  {
    // TODO should probably update the body control from hardware measurements whenever torque is enabled
    d_bodyControl->updateFromHardwareState(hw);
    State::make<BodyControlState>(d_bodyControl, d_cycleNumber);
    d_readYet = true;
  }

  t.enter("Observers");
  State::callbackObservers(ThreadId::MotionLoop, t);
  t.exit();
}

shared_ptr<HardwareState const> MotionLoop::readHardwareState(SequentialTimer& t)
{
  ASSERT(d_haveBody);

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
    log::warning("MotionLoop::process") << "CM730 bulk read failed (" << CM730::getCommResultName(res) << ")";
    onReadFailure(++d_consecutiveReadFailureCount);
    return nullptr;
  }

  if (d_consecutiveReadFailureCount)
    d_consecutiveReadFailureCount--;

  auto cm730Snapshot = make_unique<CM730Snapshot const>(d_dynamicBulkRead->getBulkReadData(CM730::ID_CM));

  auto mx28Snapshots = vector<unique_ptr<MX28Snapshot const>>();
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    mx28Snapshots.push_back(make_unique<MX28Snapshot const>(jointId, d_dynamicBulkRead->getBulkReadData(jointId), d_offsets[jointId]));

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
  ASSERT(d_haveBody);

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

  State::make<StaticHardwareState>(cm730State, mx28States);
  return true;
}

shared_ptr<HardwareState const> MotionLoop::readHardwareStateFake(SequentialTimer& t)
{
  ASSERT(!d_haveBody);

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
    State::make<StaticHardwareState>(cm730State, mx28States);

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
