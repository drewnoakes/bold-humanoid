#pragma once

#include <string>

namespace bold
{
  // TODO rename class as ThreadUtil and enum as ThreadUtil

  enum class ThreadIds
  {
    MotionLoop = 1,
    ThinkLoop = 2,
    DataStreamer = 3
  };

  class ThreadUtil
  {
  public:
    static void setThreadId(ThreadIds threadId) { d_threadId = threadId; }
    static ThreadIds getThreadId() { return d_threadId; }

    static bool isMotionLoopThread();
    static bool isThinkLoopThread();
    static bool isDataStreamerThread();

    static std::string getThreadName();

  private:
    static thread_local ThreadIds d_threadId;
  };
}
