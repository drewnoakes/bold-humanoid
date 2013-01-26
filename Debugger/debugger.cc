#include "debugger.hh"

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include <time.h>
#include <stdio.h>

using namespace Robot;
using namespace std;
using namespace bold;

double Debugger::getSeconds(timestamp_t startedAt)
{
  auto now = getTimestamp();
  return (now - startedAt) / 1000000.0L;
}

Debugger::timestamp_t Debugger::getTimestamp()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_usec + (Debugger::timestamp_t)now.tv_sec * 1000000;
}

void Debugger::timeImageProcessing(timestamp_t startedAt)
{
  double millis = getSeconds(startedAt) * 1000.0;
  bool isOverThreshold = millis > d_imageProcessingThresholdMillis;
  fprintf(isOverThreshold ? stderr : stdout, "Image processed in %.1fms\n", millis);
  d_isImageProcessingSlow = isOverThreshold;
}

void Debugger::printTime(timestamp_t startedAt, std::string const& description)
{
  double millis = getSeconds(startedAt) * 1000.0;
  fprintf(stdout, "%s in %.1fms\n", description.c_str(), millis);
}
void Debugger::setIsBallObserved(bool isBallObserved)
{
  d_isBallObserved = isBallObserved;
}

void Debugger::update(Robot::CM730& cm730)
{
  int value = 0;

  if (d_isBallObserved)
    value |= LED_RED;

  if (d_isImageProcessingSlow)
    value |= LED_BLUE;

  if (value != d_lastLEDValue)
  {
    cm730.WriteByte(Robot::CM730::P_LED_PANNEL, value, NULL);
    d_lastLEDValue = value;
  }
}
