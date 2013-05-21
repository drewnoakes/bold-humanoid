#include "clock.ih"

double Clock::getMillis()
{
  return timeStampToMillis(getTimestamp());
}

double Clock::getSeconds()
{
  return timeStampToSeconds(getTimestamp());
}

double Clock::getSecondsSince(Timestamp since)
{
  return timeStampToSeconds(getTimestamp() - since);
}