#include "clock.ih"

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
  return timestampToSeconds(getTimestamp() - since);
}

double Clock::getMillisSince(Timestamp since)
{
  return timestampToMillis(getTimestamp() - since);
}