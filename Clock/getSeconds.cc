#include "clock.ih"

double Clock::getSeconds()
{
  return getTimestamp() / 1e6;
}

double Clock::getSeconds(Timestamp since)
{
  return (getTimestamp() - since) / 1e6;
}