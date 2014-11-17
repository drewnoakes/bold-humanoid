#pragma once

#include <string>

namespace bold
{
  enum class ThreadId
  {
    Main = 1,
    MotionLoop = 2,
    ThinkLoop = 3,
    DataStreamer = 4
  };

  class ThreadUtil
  {
  public:
    static ThreadId getThreadId() { return d_threadId; }
    static void setThreadId(ThreadId threadId);

    static bool isMotionLoopThread(bool logError = true);
    static bool isThinkLoopThread(bool logError = true);
    static bool isDataStreamerThread(bool logError = true);

    static std::string getThreadName();

  private:
    static thread_local ThreadId d_threadId;
  };
}
