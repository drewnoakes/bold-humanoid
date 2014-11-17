#include "loop.hh"
#include "log.hh"

using namespace bold;
using namespace std;

Loop::Loop(std::string loopName)
  : d_loopName(loopName)
{}

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

  // Initialise default thread attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  // Set the scheduling policy as 'RR'
  int error = pthread_attr_setschedpolicy(&attr, SCHED_RR);
  if (error != 0)
  {
    log::error(d_loopName) << "Error setting thread scheduling policy as RR: " << error;
    return false;
  }

  // Set the scheduler inheritance (no inheritance)
  error = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  if (error != 0)
  {
    log::error(d_loopName) << "Error setting thread scheduler inheritence as explicit: " << error;
    return false;
  }

  // Set the thread as having real-time priority (requires elevated permissions)
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = 31;
  error = pthread_attr_setschedparam(&attr, &param);
  if (error != 0)
  {
    log::error(d_loopName) << "Error setting thread priority as realtime: " << error;
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
