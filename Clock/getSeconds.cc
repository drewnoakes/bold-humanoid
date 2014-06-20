#include "clock.ih"

#include "../util/assert.hh"

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
