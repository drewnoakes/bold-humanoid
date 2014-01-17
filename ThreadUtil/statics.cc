#include "threadutil.hh"

#include "../util/log.hh"

#include <sstream>
#include <iostream>

using namespace bold;
using namespace std;

thread_local ThreadIds ThreadUtil::d_threadId;

std::string ThreadUtil::getThreadName()
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

bool ThreadUtil::isMotionLoopThread()
{
  if (d_threadId == ThreadIds::MotionLoop)
    return true;
  log::error("ThreadUtil::isMotionLoopThread") << "Expected Motion Loop thread but was: " << getThreadName();
  return false;
}

bool ThreadUtil::isThinkLoopThread()
{
  if (d_threadId == ThreadIds::ThinkLoop)
    return true;
  log::error("ThreadUtil::isThinkLoopThread") << "Expected Think Loop thread but was: " << getThreadName();
  return false;
}

bool ThreadUtil::isDataStreamerThread()
{
  if (d_threadId == ThreadIds::DataStreamer)
    return true;
  log::error("ThreadUtil::isDataStreamerThread") << "Expected Data Streamer thread but was: " << getThreadName();
  return false;
}
