#include "threadid.hh"

#include "../util/log.hh"

#include <sstream>
#include <iostream>

using namespace bold;
using namespace std;

thread_local ThreadIds ThreadId::d_threadId;

std::string ThreadId::getThreadName()
{
  switch (d_threadId)
  {
    case ThreadIds::MotionLoop: return "Motion Loop";
    case ThreadIds::ThinkLoop: return "Think Loop";
    case ThreadIds::DataStreamer: return "Data Streamer";
    default:
      stringstream s;
      s << "Unknown (" << (int)d_threadId << ")";
      return s.str();
  }
}

bool ThreadId::isMotionLoopThread()
{
  if (d_threadId == ThreadIds::MotionLoop)
    return true;
  log::error("ThreadId::isMotionLoopThread") << "Expected Motion Loop thread but was: " << getThreadName();
  return false;
}

bool ThreadId::isThinkLoopThread()
{
  if (d_threadId == ThreadIds::ThinkLoop)
    return true;
  log::error("ThreadId::isThinkLoopThread") << "Expected Think Loop thread but was: " << getThreadName();
  return false;
}

bool ThreadId::isDataStreamerThread()
{
  if (d_threadId == ThreadIds::DataStreamer)
    return true;
  log::error("ThreadId::isDataStreamerThread") << "Expected Data Streamer thread but was: " << getThreadName();
  return false;
}
