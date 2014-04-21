#include "log.hh"

#include <cassert>
#include <unistd.h>

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
  bool omitColour = d_level == LogLevel::Error ? isStdErrRedirected (): isStdOutRedirected();

  if (omitColour)
  {
    if (d_scope.size())
      ostream << "[" << d_scope << "] ";

    ostream << d_message->str() << endl;
  }
  else
  {
    if (d_scope.size())
      ostream << ccolor::fore::lightblue << "[" << d_scope << "] ";

    ostream << "\033[" << fgColor << ";" << bgColor << "m"
            << d_message->str()
            << "\033[00m" << endl;
  }
}

bool log::isStdOutRedirected()
{
  static bool isStdOutRedirected = isatty(STDOUT_FILENO) == 0;
  return isStdOutRedirected;
}

bool log::isStdErrRedirected()
{
  static bool isStdErrRedirected = isatty(STDERR_FILENO) == 0;
  return isStdErrRedirected;
}
