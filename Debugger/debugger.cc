#include "debugger.hh"

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include <time.h>
#include <stdio.h>

using namespace Robot;
using namespace std;
using namespace bold;

const double Debugger::getSeconds(timestamp_t const& startedAt)
{
  auto now = getTimestamp();
  return (now - startedAt) / 1000000.0L;
}

const Debugger::timestamp_t Debugger::getTimestamp()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_usec + (Debugger::timestamp_t)now.tv_sec * 1000000;
}

void Debugger::timeImageCapture(timestamp_t const& startedAt)
{
  d_lastImageCaptureTimeMillis = printTime(startedAt, "Captured %4.2f ");
}

void Debugger::timeImageProcessing(timestamp_t const& startedAt)
{
  d_lastImageProcessTimeMillis = printTime(startedAt, "Processed %4.2f\n");
  bool isOverThreshold = d_lastImageProcessTimeMillis > d_imageProcessingThresholdMillis;
  d_isImageProcessingSlow = isOverThreshold;
}

const double Debugger::printTime(timestamp_t const& startedAt, std::string const& format)
{
  double millis = getSeconds(startedAt) * 1000.0;
  fprintf(stdout, format.c_str(), millis);
  return millis;
}

void Debugger::setIsBallObserved(bool const& isBallObserved)
{
  d_isBallObserved = isBallObserved;
}

void Debugger::setGoalObservationCount(int const& goalObservationCount)
{
  d_isTwoGoalPostsObserved = goalObservationCount >= 2;
}

void Debugger::update(Robot::CM730& cm730)
{
  //
  // Update LED statuses
  //
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

  //
  // Update websocket data
  //
  if (d_streamer != nullptr)
  {
    // TODO include think loop time
//    d_streamer->setTimings(d_lastImageCaptureTimeMillis, d_lastImageProcessTimeMillis);

    d_streamer->update();
  }
}

void Debugger::setGameControlData(RoboCupGameControlData const& gameControlData)
{
  // TODO do something useful with this information
  cout << "GAME CONTROL DATA RECEIVED: " << gameControlData.secsRemaining << endl;
  if (d_streamer != nullptr)
  {
//    d_streamer->updateGameControlData(gameControlData);
  }
}