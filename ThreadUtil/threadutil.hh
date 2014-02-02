#pragma once

#include <string>

namespace bold
{
  // TODO rename class as ThreadUtil and enum as ThreadUtil

  enum class ThreadId
  {
    MotionLoop = 1,
    ThinkLoop = 2,
    DataStreamer = 3
  };

  class ThreadUtil
  {
  public:
    static ThreadId getThreadId() { return d_threadId; }
    static void setThreadId(ThreadId threadId);

    static bool isMotionLoopThread();
    static bool isThinkLoopThread();
    static bool isDataStreamerThread();

    static std::string getThreadName();

  private:
    static thread_local ThreadId d_threadId;
  };
}
