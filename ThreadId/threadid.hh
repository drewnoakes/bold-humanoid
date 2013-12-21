#pragma once

#include <string>

namespace bold
{
  class ThreadId
  {
  public:
    enum ThreadIds
    {
      MotionLoop = 1,
      ThinkLoop = 2,
      DataStreamer = 3
    };

    static void setThreadId(int threadId) { d_threadId = threadId; }
    static int getThreadId() { return d_threadId; }

    static bool isMotionLoopThread();
    static bool isThinkLoopThread();
    static bool isDataStreamerThread();

    static std::string getThreadName();

  private:
    static thread_local int d_threadId;
  };
}
