#include "threadid.hh"

#include <sstream>

using namespace bold;

thread_local int ThreadId::d_threadId;

std::string ThreadId::getThreadName()
{
  switch (d_threadId)
  {
    case ThreadIds::MotionLoop: return "Motion Loop";
    case ThreadIds::ThinkLoop: return "Think Loop";
    default:
      std::stringstream s;
      s << "Unknown (" << d_threadId << ")";
      return s.str();
  }
}
