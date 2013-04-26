#include "clock.ih"

Clock::Timestamp Clock::getTimestamp()
{
  struct timeval now;
  gettimeofday(&now, 0);
  return now.tv_usec + (now.tv_sec * 1000000);
}
