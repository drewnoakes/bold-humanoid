#include "loop.hh"
#include "log.hh"

using namespace bold;
using namespace std;

Loop::Loop(std::string loopName, int schedulePolicy, int priority)
  : d_cycleNumber(0), 
    d_lastFps(0),
    d_loopName(loopName),
    d_isRunning(false),
    d_isStopRequested(false),
    d_schedulePolicy(schedulePolicy),
    d_priority(priority)
{
  if (d_priority < 0)
    d_priority = sched_get_priority_max(schedulePolicy);

  ASSERT(d_priority >= sched_get_priority_min(schedulePolicy));
  ASSERT(d_priority <= sched_get_priority_max(schedulePolicy));
}

Loop::~Loop()
{
  if (d_isRunning && !d_isStopRequested)
    stop();
}

bool Loop::start()
{
  log::verbose(d_loopName) << "Starting";

  if (d_isRunning)
  {
    log::error(d_loopName) << "Loop is already running";
    return false;
  }

  // For information on Linux thread scheduling, see: man sched_setscheduler

  // Policy
  //   Normal (static priority of zero)
  //     SCHED_OTHER (standard round-robin)
  //     SCHED_BATCH (for batch processing)
  //     SCHED_IDLE (for very low priority background tasks)
  //   Real-time (priority from 1-99)
  //     SCHED_FIFO (first in, first out, no time-slicing, always preempts priority zero)
  //     SCHED_RR (round robin, like FIFO except runs only for time slice)
  //
  // Scheduler executes highest priority thread
  // Policy only applies between threads of equal priority
  // Threads preempted by threads of higher priority

  // Initialise default thread attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  int error;

  // Set the scheduler inheritance (no inheritance)
  error = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  if (error != 0)
  {
    log::error(d_loopName) << "Error setting thread scheduler inheritence as explicit: " << error;
    return false;
  }

  // Set the scheduling policy
  error = pthread_attr_setschedpolicy(&attr, d_schedulePolicy);
  if (error != 0)
  {
    log::error(d_loopName) << "Error setting thread scheduling policy: " << error;
    return false;
  }

  // Set the thread priority
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = d_priority;
  error = pthread_attr_setschedparam(&attr, &param);
  if (error != 0)
  {
    log::error(d_loopName) << "Error setting thread priority: " << error;
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
    log::error(d_loopName) << "Error starting thread: " << error;
    return false;
  }

  d_isRunning = true;
  return true;
}

void Loop::stop()
{
  if (!d_isRunning || d_isStopRequested)
    return;

  log::verbose(d_loopName) << "Stopping";

  // set the flag to end the thread
  d_isStopRequested = true;

  // wait for the thread to end
  if (pthread_join(d_thread, NULL) != 0)
    exit(EXIT_FAILURE);

  log::info(d_loopName) << "Stopped";

  d_isStopRequested = false;
  d_isRunning = false;
}

void* Loop::threadMethod(void* param)
{
  auto loop = static_cast<Loop*>(param);

  log::info(loop->d_loopName) << "Started";

  loop->onLoopStart();

  while (!loop->d_isStopRequested)
  {
    loop->onStep(loop->d_cycleNumber);

    loop->d_lastFps = loop->d_fpsCounter.next();
    loop->d_cycleNumber++;

    loop->d_cycleNumber++;
  }

  log::verbose(loop->d_loopName) << "Stopping";

  loop->onStopped();

  log::info(loop->d_loopName) << "Stopped";

  pthread_exit(nullptr);
}

void LoopRegulator::start()
{
  clock_gettime(CLOCK_MONOTONIC, &d_nextTime);
}

///////////////////////////////////////////////////////////////////////////////

void LoopRegulator::setIntervalMicroseconds(unsigned intervalMicroseconds)
{
  clock_gettime(CLOCK_MONOTONIC, &d_nextTime);
  d_intervalMicroseconds = intervalMicroseconds;
}

void LoopRegulator::wait()
{
  // NOTE this will always increment by the interval, even if something stalled
  d_nextTime.tv_sec += (d_nextTime.tv_nsec + d_intervalMicroseconds * 1000) / 1000000000;
  d_nextTime.tv_nsec = (d_nextTime.tv_nsec + d_intervalMicroseconds * 1000) % 1000000000;

  int sleepResult = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &d_nextTime, nullptr);

  if (sleepResult != 0)
  {
    if (sleepResult == EINTR)
      log::warning("LoopRegulator::wait") << "Sleep interrupted";
    else
      log::warning("LoopRegulator::wait") << "clock_nanosleep returned error code: " << sleepResult;
  }
}
