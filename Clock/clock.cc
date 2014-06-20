#include "clock.hh"

#include "../util/assert.hh"

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
  ASSERT(now > since);
  return timestampToSeconds(now - since);
}

double Clock::getMillisSince(Timestamp since)
{
  Timestamp now = getTimestamp();
  ASSERT(now > since);
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
