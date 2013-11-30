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
      ThinkLoop = 2
    };

    static void setThreadId(int threadId) { d_threadId = threadId; }
    static int getThreadId() { return d_threadId; }

    static bool isMotionLoopThread();
    static bool isThinkLoopThread();

    static std::string getThreadName();

  private:
    static thread_local int d_threadId;
  };
}
