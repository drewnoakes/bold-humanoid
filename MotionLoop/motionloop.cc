#include "motionloop.hh"

#include "../AgentState/agentstate.hh"
#include "../BodyControl/bodycontrol.hh"
#include "../CM730Snapshot/cm730snapshot.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"

#include <time.h>
#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

MotionLoop::MotionLoop(shared_ptr<CM730> cm730)
: d_isStarted(false),
  d_stopRequested(false),
  d_loopDurationMillis(8),
  d_readYet(false)
{
  d_cm730 = cm730;
  d_bodyControl = make_shared<BodyControl>();
  d_dynamicBulkRead = make_shared<BulkRead>(CM730::P_DXL_POWER, CM730::P_VOLTAGE,
                                            MX28::P_PRESENT_POSITION_L, MX28::P_PRESENT_TEMPERATURE);

  for (int i = 0; i < JointControl::NUMBER_OF_JOINTS; i++)
    d_offsets[i] = 0;
}

MotionLoop::~MotionLoop()
{
  stop();
}

bool MotionLoop::start()
{
  cout << "[MotionLoop::start] Starting" << endl;

  d_readYet = false;
  
  // Initialise default thread attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  // Set the scheduling policy as 'RR'
  int error = pthread_attr_setschedpolicy(&attr, SCHED_RR);
  if (error != 0)
  {
    cerr << "[MotionLoop::start] Error setting thread scheduling policy as RR: " << error << endl;
    return false;
  }

  // Set the scheduler inheritence (no inheritance)
  error = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  if (error != 0)
  {
    cerr << "[MotionLoop::start] Error setting thread scheduler inheritence as explicit: " << error << endl;
    return false;
  }

  // Set the thread as having real-time priority (requires elevated permissions)
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = 31;
  error = pthread_attr_setschedparam(&attr, &param);
  if (error != 0)
  {
    cerr << "[MotionLoop::start] Error setting thread priority as realtime: " << error << endl;
    return false;
  }

  // Create and start the thread
  error = pthread_create(&d_thread, &attr, threadMethod, this);
  if (error != 0)
  {
    cerr << "[MotionLoop::start] Error starting thread: " << error << endl;
    return false;
  }

  d_isStarted = true;
  return true;
}

void MotionLoop::stop()
{
  cout << "[MotionLoop::stop] Stopping" << endl;

  if (!d_isStarted)
    return;

  // set the flag to end the thread
  d_stopRequested = true;

  // wait for the thread to end
  int error;
  if ((error = pthread_join(d_thread, NULL)) != 0)
    exit(-1);

  cout << "[MotionLoop::stop] Stopped" << endl;

  d_stopRequested = false;
  d_isStarted = false;
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

void *MotionLoop::threadMethod(void *param)
{
  cout << "[MotionLoop::threadMethod] Starting MotionLoop thread" << endl;

  MotionLoop *loop = (MotionLoop*)param;
  static struct timespec next_time;
  clock_gettime(CLOCK_MONOTONIC, &next_time);

  while (!loop->d_stopRequested)
  {
    next_time.tv_sec += (next_time.tv_nsec + loop->d_loopDurationMillis * 1000000) / 1000000000;
    next_time.tv_nsec = (next_time.tv_nsec + loop->d_loopDurationMillis * 1000000) % 1000000000;

    loop->step();

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
  }

  cout << "[MotionLoop::threadMethod] Exiting" << endl;

  pthread_exit(NULL);
}

void MotionLoop::step()
{
  if (d_readYet)
  {
    //
    // LET MOTION MODULES UPDATE BODY CONTROL
    //

    // TODO only step modules that are in use
    for (auto const& module : d_modules)
    {
      auto jointSelection = JointSelection(true, true, true);

      module->step(jointSelection);
    }

    // TODO apply body section updates via modules, as appropriate

    //
    // WRITE UPDATE
    //

    int dirtyDeviceCount = 0;
    int minAddress = MX28::P_D_GAIN;
    int maxAddress = MX28::P_GOAL_POSITION_H;
    for (shared_ptr<JointControl> joint : d_bodyControl->getJoints())
    {
      if (joint->isDirty())
      {
        dirtyDeviceCount++;
        // TODO find real min/max addresses
  //       minAddress = min(minAddress, joint->minAddress());
  //       maxAddress = max(maxAddress, joint->maxAddress());
      }
    }

    if (dirtyDeviceCount > 0)
    {
      // Prepare the parameters of a SyncWrite instruction
      int bytesPerDevice = 1 + maxAddress - minAddress + 1;
      int parameters[dirtyDeviceCount * bytesPerDevice];
      int n = 0;
      for (shared_ptr<JointControl> joint : d_bodyControl->getJoints())
      {
        if (joint->isDirty())
        {
          // Specify the goal position, and apply any calibration offset
          int goalPosition = joint->getValue() + d_offsets[joint->getId()];

          parameters[n++] = joint->getId();

          // Values to map to min/max address range
          // TODO only include between min/max addresses
          parameters[n++] = joint->getDGain();
          parameters[n++] = joint->getIGain();
          parameters[n++] = joint->getPGain();
          parameters[n++] = 0; // reserved
          parameters[n++] = CM730::getLowByte(goalPosition);
          parameters[n++] = CM730::getHighByte(goalPosition);

          joint->clearDirty();
        }
      }

      //
      // Send the SyncWrite message, if anything changed
      //

      if (dirtyDeviceCount > 0)
      {
        d_cm730->syncWrite(minAddress, bytesPerDevice, dirtyDeviceCount, parameters);
      }
    }
  }

  //
  // READ DATA
  //

  CommResult res = d_cm730->bulkRead(d_dynamicBulkRead);

  if (res != CommResult::SUCCESS)
  {
    // TODO set the 'Hardware' state as failing in AgentState, and broadcast error status to clients
    cerr << "[MotionLoop::process] Bulk read failed -- skipping update of HardwareState" << endl;
    return;
  }

  auto cm730Snapshot = make_shared<CM730Snapshot>(d_dynamicBulkRead->getBulkReadData(CM730::ID_CM));

  auto mx28Snapshots = vector<shared_ptr<MX28Snapshot const>>();
  mx28Snapshots.push_back(nullptr); // padding as joints start at 1
  for (unsigned jointId = 1; jointId < JointControl::NUMBER_OF_JOINTS; jointId++)
  {
    auto mx28 = make_shared<MX28Snapshot>(d_dynamicBulkRead->getBulkReadData(jointId), jointId);
    mx28Snapshots.push_back(mx28);
  }

  //
  // UPDATE HARDWARE STATE
  //

  auto rxBytes = d_cm730->getReceivedByteCount();
  auto txBytes = d_cm730->getTransmittedByteCount();

  AgentState::getInstance().set(make_shared<HardwareState const>(cm730Snapshot, mx28Snapshots, rxBytes, txBytes));

  //
  // UPDATE BODYSTATE
  //

  // TODO implement this as an observer of HardwareState
  double angles[JointControl::NUMBER_OF_JOINTS];
  for (unsigned jointId = 1; jointId < JointControl::NUMBER_OF_JOINTS; jointId++)
  {
    angles[jointId] = mx28Snapshots[jointId]->presentPosition;
  }

  AgentState::getInstance().set(make_shared<BodyState const>(angles));
  
  d_readYet = true;
}
