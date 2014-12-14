#include "log.hh"

#include <unistd.h>

#include "../StateObject/GameState/gamestate.hh"
#include "../State/state.hh"

#include "ccolor.hh"

using namespace bold;
using namespace std;

LogLevel log::minLevel = LogLevel::Verbose;
bool log::logGameState = false;

vector<unique_ptr<LogAppender>> log::d_appenders;

log::~log()
{
  if (!d_message)
    return;

  for (auto& appender : d_appenders)
    appender->append(d_level, d_scope, d_message);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void writeLogTimestamp(ostream& o, bool isConsole)
{
  time_t now = time(0);
  tm tstruct = *localtime(&now);
  char buf[80];
  // http://en.cppreference.com/w/cpp/chrono/c/strftime
  strftime(buf, sizeof(buf), isConsole ? "%T" : "%X", &tstruct);
  o << buf << " ";
}

void writeGameState(ostream& o)
{
  // Ensure the game state has been registered
  if (!log::logGameState)
    return;

  auto game = State::get<GameState>();

  if (!game)
    return;

  auto result = game->getGameResult();
  if (result != GameResult::Undecided)
  {
    o << getGameResultName(result)
      << " "
      << (int)game->getMyTeam().getTeamNumber() << "v" << (int)game->getOpponentTeam().getTeamNumber()
      << " "
      << (int)game->getMyTeam().getScore() << ":" << (int)game->getOpponentTeam().getScore();
  }
  else
  {
    auto mode = game->getPlayMode();
    o << (int)game->getMyTeam().getTeamNumber() << "v" << (int)game->getOpponentTeam().getTeamNumber()
      << " "
      << (int)game->getMyTeam().getScore() << ":" << (int)game->getOpponentTeam().getScore()
      << " "
      << getPlayModeName(mode)
      << " ";
    if (game->isPenaltyShootout())
    {
      o << "penalties ";
    }
    else
    {
      o << (game->isFirstHalf() ? "first" : "second");
      if (game->isOvertime())
        o << "-extra";
      o << " ";
    }

    o << game->getSecondsRemaining();
    auto secondary = game->getSecondaryTime();
    if (secondary != 0)
      o << "/" << secondary;
  }

  o << " ";
}

void writeLevelShortName(ostream& o, bold::LogLevel level)
{
  switch (level)
  {
    case LogLevel::Trace:   o << "trc "; break;
    case LogLevel::Verbose: o << "vrb "; break;
    case LogLevel::Info:    o << "inf "; break;
    case LogLevel::Warning: o << "WRN "; break;
    case LogLevel::Error:   o << "ERR "; break;
    default:                o << "??? "; break;
  }
}

void ConsoleLogAppender::append(LogLevel const& level, string scope, unique_ptr<ostringstream> const& message)
{
  int fgColor = 39;
  int bgColor = 49;

  switch (level)
  {
    case LogLevel::Trace:
      fgColor = 90;
      break;
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

  auto& ostream = level == LogLevel::Error ? cerr : cout;
  bool isRedirected = level == LogLevel::Error ? isStdErrRedirected() : isStdOutRedirected();

  if (isRedirected)
  {
    writeLogTimestamp(ostream, /*isConsole*/ false);
    writeGameState(ostream);
    writeLevelShortName(ostream, level);

    if (scope.size())
      ostream << "[" << scope << "] ";

    ostream << message->str() << endl;
  }
  else
  {
    ostream << ccolor::fore::lightblack;
    writeLogTimestamp(ostream, /*isConsole*/ true);

    if (scope.size())
      ostream << ccolor::fore::lightblue << "[" << scope << "] ";

    ostream << "\033[" << fgColor << ";" << bgColor << "m"
      << message->str()
      << "\033[00m" << endl;
  }
}

bool ConsoleLogAppender::isStdOutRedirected()
{
  static bool isStdOutRedirected = isatty(STDOUT_FILENO) == 0;
  return isStdOutRedirected;
}

bool ConsoleLogAppender::isStdErrRedirected()
{
  static bool isStdErrRedirected = isatty(STDERR_FILENO) == 0;
  return isStdErrRedirected;
}
