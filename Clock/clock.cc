#include "clock.hh"

#include "../util/assert.hh"
#include "../util/log.hh"

#include <string.h>
#include <sys/time.h>
#include <math.h>

using namespace bold;

double Clock::getMillis()
{
  return timestampToMillis(getTimestamp());
}

double Clock::getSeconds()
{
  return timestampToSeconds(getTimestamp());
}

double Clock::getSecondsSince(Timestamp since)
{
  Timestamp now = getTimestamp();
#ifdef INCLUDE_ASSERTIONS
  if (now < since)
    log::warning("Clock") << "Time reversed. now=" << now << " since=" << since << " delta=" << (now-since);
#endif
  return timestampToSeconds(now - since);
}

double Clock::getMillisSince(Timestamp since)
{
  Timestamp now = getTimestamp();
#ifdef INCLUDE_ASSERTIONS
  if (now < since)
    log::warning("Clock") << "Time reversed. now=" << now << " since=" << since << " delta=" << (now-since);
#endif
  return timestampToMillis(now - since);
}

Clock::Timestamp Clock::getTimestamp()
{
  struct timeval now;
#ifdef INCLUDE_ASSERTIONS
  if (gettimeofday(&now, 0) == -1)
    log::warning("Clock::getTimestamp") << "Error returned by gettimeofday: " << strerror(errno) << " (" << errno << ")";
#endif
  return (Timestamp)now.tv_usec + ((Timestamp)now.tv_sec * (Timestamp)1000000);
}

double Clock::timestampToMillis(Timestamp timestamp)
{
  return timestamp / 1e3;
}

double Clock::timestampToSeconds(Timestamp timestamp)
{
  return timestamp / 1e6;
}

std::string Clock::describeDurationSeconds(double seconds)
{
  seconds = fabs(seconds);

  int minutes = seconds / 60;
  int hours = seconds / (60 * 60);
  int days = seconds / (60 * 60 * 24);

  std::stringstream out;

  if (days > 2)
  {
    out << days << " day" << (days == 1 ? "" : "s");
  }
  else if (minutes > 90)
  {
    out << hours << " hour" << (hours == 1 ? "" : "s");
  }
  else if (seconds > 90)
  {
    out << minutes << " minute" << (minutes == 1 ? "" : "s");
  }
  else
  {
    out << seconds << " second" << (seconds == 1 ? "" : "s");
  }

  return out.str();
}

std::string Clock::describeDurationSince(Clock::Timestamp timestamp)
{
  return describeDurationSeconds(getSecondsSince(timestamp));
}
