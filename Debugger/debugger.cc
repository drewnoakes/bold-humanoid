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

void Debugger::timeImageCapture(timestamp_t startedAt)
{
  printTime(startedAt, "Captured %4.2f ");
}

void Debugger::timeImageProcessing(timestamp_t startedAt)
{
  double millis = getSeconds(startedAt) * 1000.0;
  bool isOverThreshold = millis > d_imageProcessingThresholdMillis;
  d_isImageProcessingSlow = isOverThreshold;

  printTime(startedAt, "Processed %4.2f\n");
}

void Debugger::printTime(timestamp_t startedAt, std::string const& format)
{
  double millis = getSeconds(startedAt) * 1000.0;
  fprintf(stdout, format.c_str(), millis);
}

void Debugger::setIsBallObserved(bool isBallObserved)
{
  d_isBallObserved = isBallObserved;
}

void Debugger::setGoalObservationCount(int goalObservationCount)
{
  d_isTwoGoalPostsObserved = goalObservationCount >= 2;
}

void Debugger::update(Robot::CM730& cm730)
{
  int value = 0;

  if (d_isBallObserved)
    value |= LED_RED;

  if (d_isImageProcessingSlow)
    value |= LED_BLUE;

  if (d_isTwoGoalPostsObserved)
    value |= LED_GREEN;

  if (value != d_lastLEDValue)
  {
    cm730.WriteByte(Robot::CM730::P_LED_PANNEL, value, NULL);
    d_lastLEDValue = value;
  }
}
