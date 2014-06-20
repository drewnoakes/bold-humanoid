#include "clock.hh"

#include "../util/assert.hh"

#ifdef INCLUDE_ASSERTIONS
#include "../util/log.hh"
#endif

#include <sys/time.h>

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
  gettimeofday(&now, 0);
  return now.tv_usec + (now.tv_sec * 1000000);
}

double Clock::timestampToMillis(Timestamp timestamp)
{
  return timestamp / 1e3;
}

double Clock::timestampToSeconds(Timestamp timestamp)
{
  return timestamp / 1e6;
}
