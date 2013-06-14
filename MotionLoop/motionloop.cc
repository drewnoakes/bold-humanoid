#include "motionloop.hh"

#include "../AgentState/agentstate.hh"
#include "../BodyControl/bodycontrol.hh"
#include "../CM730/cm730.hh"
#include "../CM730Snapshot/cm730snapshot.hh"
#include "../Debugger/debugger.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/MotionTaskState/motiontaskstate.hh"
#include "../StateObject/TimingState/timingstate.hh"
#include "../ThreadId/threadid.hh"

#include <time.h>
#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

MotionLoop::MotionLoop(shared_ptr<CM730> cm730)
: d_cm730(cm730),
  d_isStarted(false),
  d_stopRequested(false),
  d_loopDurationMillis(8),
  d_readYet(false)
{
  d_bodyControl = make_shared<BodyControl>();
  d_dynamicBulkRead = make_shared<BulkRead>(CM730::P_DXL_POWER, CM730::P_VOLTAGE,
                                            MX28::P_PRESENT_POSITION_L, MX28::P_PRESENT_TEMPERATURE);

  for (uchar i = 0; i < (uchar)JointId::MAX; i++)
    d_offsets[i] = 0;
}

MotionLoop::~MotionLoop()
{
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

void *MotionLoop::threadMethod(void *param)
{
  cout << "[MotionLoop::threadMethod] Started" << endl;
  
  ThreadId::setThreadId(ThreadId::MotionLoop);

  MotionLoop *loop = (MotionLoop*)param;
  static struct timespec next_time;
  clock_gettime(CLOCK_MONOTONIC, &next_time);

  while (!loop->d_stopRequested)
  {
    // TODO this will always increment by <8ms, even if something stalled
    next_time.tv_sec += (next_time.tv_nsec + loop->d_loopDurationMillis * 1000000) / 1000000000;
    next_time.tv_nsec = (next_time.tv_nsec + loop->d_loopDurationMillis * 1000000) % 1000000000;

    SequentialTimer t;
  
    loop->step(t);
    
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
    t.timeEvent("Sleep");

    // Set timing data for the motion cycle
    AgentState::getInstance().set(make_shared<MotionTimingState const>(t.flush()));
  }

  cout << "[MotionLoop::threadMethod] Exiting" << endl;

  pthread_exit(NULL);
}

void MotionLoop::step(SequentialTimer& t)
{
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
      
      t.timeEvent("Write to CM730");
    }
  }

  //
  // READ DATA
  //

  CommResult res = d_cm730->bulkRead(d_dynamicBulkRead);
  t.timeEvent("Read from CM730");

  if (res != CommResult::SUCCESS)
  {
    // TODO if this occurs N times in a row, consider recreating the CM730 instance (perhaps someone pressed the hardware reset button)
    cerr << "[MotionLoop::process] Bulk read failed (" << CM730::getCommResultName(res) << ") -- skipping update of HardwareState" << endl;
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

  AgentState::getInstance().set(make_shared<HardwareState const>(cm730Snapshot, mx28Snapshots, rxBytes, txBytes));
  t.timeEvent("Update HardwareState");

  //
  // UPDATE BODYSTATE
  //

  // TODO implement this as an observer of HardwareState, and perform calculation on think thread, not motion thread (?)
  double angles[(uchar)JointId::MAX + 1];
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    angles[jointId] = mx28Snapshots[jointId - 1]->presentPosition;
  }

  AgentState::getInstance().set(make_shared<BodyState const>(angles));
  t.timeEvent("Update BodyState");
  
  if (!d_readYet)
  {
    d_bodyControl->updateFromHardwareState();
    d_readYet = true;
  }
}
