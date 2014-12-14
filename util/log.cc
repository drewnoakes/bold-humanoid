#include "log.hh"

#include <unistd.h>

#include "../StateObject/GameState/gamestate.hh"
#include "../State/state.hh"

#include "ccolor.hh"

using namespace bold;
using namespace std;

LogLevel log::minLevel = LogLevel::Verbose;
bool log::logGameState = false;

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

log::~log()
{
  if (!d_message)
    return;

  int fgColor = 39;
  int bgColor = 49;

  switch (d_level)
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

  auto& ostream = d_level == LogLevel::Error ? cerr : cout;
  bool isRedirected = d_level == LogLevel::Error ? isStdErrRedirected() : isStdOutRedirected();

  if (isRedirected)
  {
    writeLogTimestamp(ostream, /*isConsole*/ false);
    writeGameState(ostream);
    writeLevelShortName(ostream, d_level);

    if (d_scope.size())
      ostream << "[" << d_scope << "] ";

    ostream << d_message->str() << endl;
  }
  else
  {
    ostream << ccolor::fore::lightblack;
    writeLogTimestamp(ostream, /*isConsole*/ true);

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
