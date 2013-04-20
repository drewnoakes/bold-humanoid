#include "debugger.hh"

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include <time.h>
#include <stdio.h>

#include "../AgentState/agentstate.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"

using namespace Robot;
using namespace std;
using namespace bold;

Debugger::Debugger()
: d_lastLedFlags(0xff),
  d_eventTimings()
{}

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

const double Debugger::printTime(timestamp_t const& startedAt, std::string const& format)
{
  double millis = getSeconds(startedAt) * 1000.0;
  fprintf(stdout, format.c_str(), millis);
  return millis;
}

Debugger::timestamp_t Debugger::timeEvent(timestamp_t const& startedAt, std::string const& eventName)
{
  auto timeSeconds = getSeconds(startedAt);
  addEventTiming(EventTiming(timeSeconds, eventName));
  return getTimestamp();
}

void Debugger::addEventTiming(EventTiming const& eventTiming)
{
  d_eventTimings.push_back(eventTiming);
}

void Debugger::update(std::shared_ptr<Robot::CM730> cm730)
{
  auto const& cameraFrame = AgentState::get<CameraFrameState>();

  if (!cameraFrame)
    return;

  int ledFlags = 0;
  if (cameraFrame->getBallObservation().hasValue())
    ledFlags |= LED_RED;

//if (somethingElse)
//  value |= LED_BLUE;

  if (cameraFrame->getGoalObservations().size() > 1)
    ledFlags |= LED_GREEN;

  if (ledFlags != d_lastLedFlags)
  {
    // the value changed, so write it
    cm730->WriteByte(CM730::P_LED_PANNEL, ledFlags, NULL);
    d_lastLedFlags = ledFlags;
  }
}
