#include "threadutil.hh"

#include "../util/log.hh"

#include <sstream>
#include <pthread.h>

using namespace bold;
using namespace std;

thread_local ThreadId ThreadUtil::d_threadId;

void ThreadUtil::setThreadId(ThreadId threadId)
{
  d_threadId = threadId;

  if (pthread_setname_np(pthread_self(), getThreadName().c_str()) != 0)
    log::error("ThreadUtil::setThreadId") << "Error setting thread name";
}

std::string ThreadUtil::getThreadName()
{
  switch (d_threadId)
  {
    case ThreadId::MotionLoop: return "Motion Loop";
    case ThreadId::ThinkLoop: return "Think Loop";
    case ThreadId::DataStreamer: return "Data Streamer";
    default:
      stringstream s;
      s << "Unknown (" << (int)d_threadId << ")";
      return s.str();
  }
}

bool ThreadUtil::isMotionLoopThread()
{
  if (d_threadId == ThreadId::MotionLoop)
    return true;
  log::error("ThreadUtil::isMotionLoopThread") << "Expected Motion Loop thread but was: " << getThreadName();
  return false;
}

bool ThreadUtil::isThinkLoopThread()
{
  if (d_threadId == ThreadId::ThinkLoop)
    return true;
  log::error("ThreadUtil::isThinkLoopThread") << "Expected Think Loop thread but was: " << getThreadName();
  return false;
}

bool ThreadUtil::isDataStreamerThread()
{
  if (d_threadId == ThreadId::DataStreamer)
    return true;
  log::error("ThreadUtil::isDataStreamerThread") << "Expected Data Streamer thread but was: " << getThreadName();
  return false;
}
