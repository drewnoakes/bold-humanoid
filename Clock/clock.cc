#include "clock.hh"

#include "../util/assert.hh"

#ifdef INCLUDE_ASSERTIONS
#include "../util/log.hh"
#endif

#include <string.h>
#include <sys/time.h>

using namespace bold;

typedef unsigned long long ullong;

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
  if (gettimeofday(&now, 0) == -1)
    log::warning("Clock::getTimestamp") << "Error returned by gettimeofday: " << strerror(errno) << " (" << errno << ")";
  return (ullong)now.tv_usec + ((ullong)now.tv_sec * (ullong)1000000);
}

double Clock::timestampToMillis(Timestamp timestamp)
{
  return timestamp / 1e3;
}

double Clock::timestampToSeconds(Timestamp timestamp)
{
  return timestamp / 1e6;
}
