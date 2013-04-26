#include "clock.ih"

double Clock::getSeconds()
{
  return getTimestamp() / 1e6;
}