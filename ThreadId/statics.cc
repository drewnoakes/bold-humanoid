#include "threadid.hh"

#include "../util/ccolor.hh"

#include <sstream>
#include <iostream>

using namespace bold;
using namespace std;

thread_local int ThreadId::d_threadId;

std::string ThreadId::getThreadName()
{
  switch (d_threadId)
  {
    case ThreadIds::MotionLoop: return "Motion Loop";
    case ThreadIds::ThinkLoop: return "Think Loop";
    default:
      stringstream s;
      s << "Unknown (" << d_threadId << ")";
      return s.str();
  }
}

bool ThreadId::isMotionLoopThread()
{
  if (d_threadId == MotionLoop)
    return true;
  cerr << ccolor::error << "[ThreadId::isMotionLoopThread] Expected Motion Loop thread but was: " << getThreadName() << ccolor::reset << endl;
  return false;
}

bool ThreadId::isThinkLoopThread()
{
  if (d_threadId == ThinkLoop)
    return true;
  cerr << ccolor::error << "[ThreadId::isThinkLoopThread] Expected Think Loop thread but was: " << getThreadName() << ccolor::reset << endl;
  return false;
}
