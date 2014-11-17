#pragma once

#include <pthread.h>
#include <string>

#include "fps.hh"

typedef unsigned long ulong;

namespace bold
{
  class Loop
  {
  public:
    Loop(std::string loopName);

    ~Loop();

//    void setFifo();
//    void setRealtime();

    bool start();

    void stop();

    ulong getCycleNumber() const { return d_cycleNumber; }

    double getFps() const { return d_lastFps; }

    bool isRunning() const { return d_isRunning; }

  protected:
    virtual void onLoopStart() {}
    virtual void onStep(ulong cycleNumber) = 0;
    virtual void onStopped() {}

  private:
    /** Governs the thread's lifetime and operation. */
    static void *threadMethod(void* param);

    ulong d_cycleNumber;
    FPS<100> d_fpsCounter;
    double d_lastFps;
    pthread_t d_thread;
    std::string d_loopName;
    bool d_isRunning;
    bool d_isStopRequested;
  };

  class LoopRegulator
  {
  public:
    void setIntervalMicroseconds(unsigned intervalMicroseconds);

    void start();

    void wait();

  private:
    timespec d_nextTime;
    unsigned d_intervalMicroseconds;
  };
}
