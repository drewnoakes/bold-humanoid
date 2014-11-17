#include "motionloop.hh"

#include "../BodyControl/bodycontrol.hh"
#include "../BodyModel/DarwinBodyModel/darwinbodymodel.hh"
#include "../CM730/cm730.hh"
#include "../CM730Platform/CM730Linux/cm730linux.hh"
#include "../CM730Snapshot/cm730snapshot.hh"
#include "../Config/config.hh"
#include "../DebugControl/debugcontrol.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../State/state.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/BodyControlState/bodycontrolstate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/StaticHardwareState/statichardwarestate.hh"
#include "../StateObject/TimingState/timingstate.hh"
#include "../StateObject/MotionTaskState/motiontaskstate.hh"
#include "../Voice/voice.hh"

#include <rapidjson/prettywriter.h>
#include <rapidjson/filestream.h>

using namespace bold;
using namespace std;

MotionLoop::MotionLoop(shared_ptr<DebugControl> debugControl)
  : Loop("Motion Loop"),
    d_debugControl(debugControl),
    d_haveBody(false),
    d_readYet(false),
    d_staticHardwareStateUpdateNeeded(true)
{
  d_loopRegulator.setIntervalMicroseconds(MotionModule::TIME_UNIT * 1000);

  d_bodyControl = make_shared<BodyControl>();
  d_bodyModel = make_shared<DarwinBodyModel>();

  d_dynamicBulkRead = make_unique<BulkRead>(
    CM730::P_DXL_POWER, CM730::P_VOLTAGE,
    MX28::P_PRESENT_POSITION_L, MX28::P_PRESENT_TEMPERATURE);

  d_staticBulkRead = make_unique<BulkRead>(
    CM730::P_MODEL_NUMBER_L, CM730::P_RETURN_LEVEL,
    MX28::P_MODEL_NUMBER_L, MX28::P_LOCK);

  Config::addAction("hardware.query-static-hardware-state", "Query static HW state", [this] { d_staticHardwareStateUpdateNeeded = true; });
//   Config::addAction("hardware.cm730-power-on",  "CM730 On",  [this] { d_powerChangeToValue = true;  d_powerChangeNeeded = true; });
//   Config::addAction("hardware.cm730-power-off", "CM730 Off", [this] { d_powerChangeToValue = false; d_powerChangeNeeded = true; });
  Config::addAction("hardware.motor-torque-on",  "Torque On",  [this] { d_torqueChangeToValue = true;  d_torqueChangeNeeded = true; });
  Config::addAction("hardware.motor-torque-off", "Torque Off", [this] { d_torqueChangeToValue = false; d_torqueChangeNeeded = true; });

  d_offsets[0] = 0;
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    stringstream path;
    path << "hardware.offsets." << JointName::getJsonName(jointId);
    Config::getSetting<int>(path.str())->track([this, jointId](int value)
    {
      d_offsets[jointId] = value;
      d_bodyControl->getJoint((JointId)jointId)->notifyOffsetChanged();
    });
  }
}

void MotionLoop::addMotionModule(shared_ptr<MotionModule> const& module)
{
  ASSERT(module);
  d_motionModules.push_back(module);
}

void MotionLoop::addCommsModule(shared_ptr<CM730CommsModule> const& module)
{
  ASSERT(module);
  d_commsModules.push_back(module);
}

void MotionLoop::onLoopStart()
{
  // Connect to hardware subcontroller
  auto cm730DevicePath = Config::getStaticValue<string>("hardware.cm730-path");
  log::info("MotionLoop::start") << "Using CM730 Device Path: " << cm730DevicePath;
  d_cm730 = unique_ptr<CM730>(new CM730(make_unique<CM730Linux>(cm730DevicePath)));

  d_readYet = false;

  ThreadUtil::setThreadId(ThreadId::MotionLoop);

  d_haveBody = d_cm730->connect();

  if (!d_haveBody)
  {
    log::warning("MotionLoop::threadMethod") << "Motion loop running without a body";
  }
  else
  {
    // Set all CM730/MX28 values to a known set of initial values before continuing
    initialiseHardwareTables();

    if (!d_cm730->torqueEnable(true))
      log::error("MotionLoop::threadMethod") << "Error enabling torque";
  }

  d_loopRegulator.start();
}

void MotionLoop::initialiseHardwareTables()
{
  ASSERT(d_haveBody);

  log::verbose("MotionLoop::initialiseHardwareTables") << "Starting";

  //
  // Helper functions
  //

  const int retryCount = 10;

  auto writeByteWithRetry = [this,retryCount](BulkReadTable const& table, uchar jointId, uchar address, uchar value)
  {
    // Don't write anything if the value is already set correctly
    if (table.readByte(address) == value)
      return;

    log::info("MotionLoop::initialiseHardwareTables") << "Writing value " << (int)value << " to EEPROM address '" << MX28::getAddressName(address) << "' of " << JointName::getEnumName(jointId);

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
          log::error("MotionLoop::initialiseHardwareTables") << "Communication problem writing " << MX28::getAddressName(address) << " to " << JointName::getEnumName(jointId) << ") after " << retryCount << " retries: " << getCommResultName(res);
          return;
        }
        usleep(50000); // 50ms
      }
    }
  };

  auto writeWordWithRetry = [this,retryCount](BulkReadTable const& table, uchar jointId, uchar address, ushort value)
  {
    // Don't write anything if the value is already set correctly
    if (table.readWord(address) == value)
      return;

    log::info("MotionLoop::initialiseHardwareTables") << "Writing value " << (int)value << " to EEPROM address '" << MX28::getAddressName(address) << "' of " << JointName::getEnumName(jointId);

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
          log::error("MotionLoop::initialiseHardwareTables") << "Communication problem writing " << MX28::getAddressName(address) << " to " << JointName::getEnumName(jointId) << ") after " << retryCount << " retries: " << getCommResultName(res);
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
      log::error("MotionLoop::initialiseHardwareTables") << "Communication problem pinging " << JointName::getEnumName(jointId) << ": " << getCommResultName(res);
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
  // Read current EEPROM values.
  // Avoid writing to EEPROM unless necessary.
  //

  BulkRead eepromBulkRead(
    CM730::P_MODEL_NUMBER_L, CM730::P_RETURN_LEVEL,
    MX28::P_MODEL_NUMBER_L, MX28::P_ALARM_SHUTDOWN);

  while (true)
  {
    auto res = d_cm730->bulkRead(&eepromBulkRead);
    if (res == CommResult::SUCCESS)
      break;
    log::warning("MotionLoop::initialiseHardwareTables") << "Communication problem (" << getCommResultName(res) << ") during bulk read. Retrying...";
  }

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
    path << "hardware.limits.angle-limits." << JointName::getJsonName(jointId);
    Range<double> rangeDegs = Config::getStaticValue<Range<double>>(path.str());

    auto const& table = eepromBulkRead.getBulkReadData(jointId);

    writeByteWithRetry(table, jointId, MX28::P_RETURN_DELAY_TIME, 0);
    writeByteWithRetry(table, jointId, MX28::P_RETURN_LEVEL, 2);
    writeWordWithRetry(table, jointId, MX28::P_CW_ANGLE_LIMIT_L, MX28::degs2Value(rangeDegs.min()));
    writeWordWithRetry(table, jointId, MX28::P_CCW_ANGLE_LIMIT_L, MX28::degs2Value(rangeDegs.max()));
    writeByteWithRetry(table, jointId, MX28::P_HIGH_LIMIT_TEMPERATURE, MX28::centigrade2Value(tempLimit));
    writeByteWithRetry(table, jointId, MX28::P_LOW_LIMIT_VOLTAGE, MX28::voltage2Value(voltageRange.min()));
    writeByteWithRetry(table, jointId, MX28::P_HIGH_LIMIT_VOLTAGE, MX28::voltage2Value(voltageRange.max()));
    writeWordWithRetry(table, jointId, MX28::P_MAX_TORQUE_L, MX28::MAX_TORQUE);
    writeByteWithRetry(table, jointId, MX28::P_ALARM_LED, alarmLed.getFlags());
    writeByteWithRetry(table, jointId, MX28::P_ALARM_SHUTDOWN, alarmShutdown.getFlags());
  }

  log::info("MotionLoop::initialiseHardwareTables") << "All MX28 data tables initialised";
}

void MotionLoop::onStep(ulong cycleNumber)
{
  SequentialTimer t;

  // Rate the limit at which we make this call to the CM730.
  if (cycleNumber % 60 == 0 && d_haveBody)
  {
    d_isCM730PowerEnabled = d_cm730->isPowerEnabled();
    t.timeEvent("Check Power");
  }

  // When unpowered, don't perform any update at all in the cycle
  if (d_haveBody && !d_isCM730PowerEnabled)
    return;

  // Don't write until we've read
  if (d_readYet)
  {
    if (d_haveBody)
    {
      if (d_powerChangeNeeded)
      {
        if (!d_cm730->powerEnable(d_powerChangeToValue))
          log::error("MotionLoop::onStep") << "Error setting power to " << d_powerChangeToValue;
        d_powerChangeNeeded = false;
      }

      if (d_torqueChangeNeeded)
      {
        if (!d_cm730->torqueEnable(d_torqueChangeToValue))
          log::error("MotionLoop::onStep") << "Error setting torque to " << d_torqueChangeToValue;
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
      State::make<BodyControlState>(d_bodyControl, cycleNumber);
      t.timeEvent("Set BodyControlState");
      d_bodyControl->clearModulation();
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

        ASSERT(bytesToWrite < std::numeric_limits<uchar>::max());

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

  State::make<BodyState>(d_bodyModel, hw, d_bodyControl, cycleNumber);
  t.timeEvent("Update BodyState");

  if (!d_readYet)
  {
    // TODO should probably update the body control from hardware measurements whenever torque is enabled
    d_bodyControl->updateFromHardwareState(hw);
    State::make<BodyControlState>(d_bodyControl, cycleNumber);
    d_readYet = true;
  }

  t.enter("Observers");
  State::callbackObservers(ThreadId::MotionLoop, t);
  t.exit();

  // Assume that comms modules cannot run when we are running without a body (debugging)
  if (d_haveBody)
  {
    t.enter("Comms");
    for (auto const& commsModule : d_commsModules)
    {
      t.enter(commsModule->getName());
      commsModule->step(d_cm730, t, cycleNumber);
      t.exit();
    }
    t.exit();
  }

  d_loopRegulator.wait();
  t.timeEvent("Sleep");

  // Set timing data for the motion cycle
  State::make<MotionTimingState>(t.flush(), cycleNumber, getFps());
}

void MotionLoop::onStopped()
{
  if (d_haveBody)
  {
    if (!d_cm730->torqueEnable(false))
      log::error("MotionLoop::threadMethod") << "Error disabling torque";
  }

  // Destroy the CM730 object on the motion thread
  d_cm730.reset();
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
          ASSERT(addrRange.contains(MX28::P_GOAL_POSITION_H));

          // Specify the goal position, and apply any modulation and calibration offset
          int goalPosition = MX28::clampValue(
            joint->getValue() +
            joint->getModulationOffset() +
            d_offsets[joint->getId()]
          );

          ushort value = MX28::clampValue(goalPosition);
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

void writeHardwareStateJsonFile()
{
  using namespace rapidjson;

  auto state = State::get<StaticHardwareState>();

  ASSERT(state);

  FILE* file = fopen("eeprom.json", "w");
  FileStream stream(file);
  PrettyWriter<FileStream> writer(stream);
  state->writeJson(writer);
  stream.Flush();
  fclose(file);
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
    {
      writeHardwareStateJsonFile();
      d_staticHardwareStateUpdateNeeded = false;
    }
    t.timeEvent("Read StaticHardwareState");
  }

  CommResult res = d_cm730->bulkRead(d_dynamicBulkRead.get());
  t.timeEvent("Read from CM730");

  if (res != CommResult::SUCCESS)
  {
    log::warning("MotionLoop::process") << "CM730 bulk read failed (" << getCommResultName(res) << ")";
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

  auto hw = make_shared<HardwareState const>(move(cm730Snapshot), move(mx28Snapshots), rxBytes, txBytes, getCycleNumber());
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
  cm730State->acc = Eigen::Vector3d(0, 0, 5);
  cm730State->accRaw = Eigen::Vector3i(512, 512, 768);
  cm730State->eyeColor = d_debugControl->getEyeColour().toRgbUnitVector();
  cm730State->foreheadColor = d_debugControl->getForeheadColour().toRgbUnitVector();
  cm730State->gyro = Eigen::Vector3d(0, 0, 0);
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

  auto hw = make_shared<HardwareState const>(unique_ptr<CM730Snapshot const>(cm730State), move(mx28States), 0, 0, getCycleNumber());
  t.timeEvent("Update HardwareState (Dummy)");
  return hw;
}
