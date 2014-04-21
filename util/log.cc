#include "log.hh"

#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include "ccolor.hh"
#include "assert.hh"

using namespace bold;
using namespace std;

LogLevel log::minLevel = LogLevel::Verbose;

std::string getLogTimestamp()
{
  time_t now = time(0);
  tm tstruct = *localtime(&now);
  char buf[80];
  // http://en.cppreference.com/w/cpp/chrono/c/strftime
  strftime(buf, sizeof(buf), "%X", &tstruct);
  return buf;
}

std::string getLevelShortName(bold::LogLevel level)
{
  switch (level)
  {
    case LogLevel::Verbose: return "vrb";
    case LogLevel::Info:    return "inf";
    case LogLevel::Warning: return "WRN";
    case LogLevel::Error:   return "ERR";
    default:                return "???";
  }
}

log::~log()
{
  if (!d_message)
    return;

  int fgColor = 39;
  int bgColor = 49;

  switch (d_level)
  {
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
  bool isRedirected = d_level == LogLevel::Error ? isStdErrRedirected (): isStdOutRedirected();

  if (isRedirected)
  {
    ostream << getLogTimestamp() << " " << getLevelShortName(d_level) << " ";

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
