#include "log.hh"

#include <cassert>

#include "ccolor.hh"

using namespace bold;
using namespace std;

LogLevel log::minLevel = LogLevel::Verbose;

log::~log()
{
  if (!d_message)
    return;
  
  int fgColor = 39;
  int bgColor = 49;

  switch (d_level) {
    case LogLevel::Verbose:
      fgColor = 37;
      break;
    case LogLevel::Info:
      break;
    case LogLevel::Warning:
      fgColor = 95;
      break;
    case LogLevel::Error:
      fgColor = 37;
      bgColor = 41;
      break;
  }

  auto& ostream = d_level == LogLevel::Error ? cerr : cout;

  if (d_scope.size())
    ostream << ccolor::fore::lightblue << "[" << d_scope << "] ";

  ostream << "\033[" << fgColor << ";" << bgColor << "m"
          << d_message->str()
          << "\033[00m" << endl;
}
