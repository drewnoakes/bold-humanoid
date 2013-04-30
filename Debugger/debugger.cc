#include "debugger.ih"

Debugger::Debugger()
: d_lastLedFlags(0xff),
  d_lastEyeInt(0),
  d_lastHeadInt(0),
  d_eventTimings(),
  d_eyeColour(0,0,0),
  d_headColour(0,0,0)
{
  // TODO intial head/eye colour not well defined
}

Clock::Timestamp Debugger::timeEvent(Clock::Timestamp const& startedAt, std::string const& eventName)
{
  auto timeSeconds = Clock::getSeconds(startedAt);
  addEventTiming(EventTiming(timeSeconds, eventName));
  return Clock::getTimestamp();
}

void Debugger::addEventTiming(EventTiming const& eventTiming)
{
  d_eventTimings.push_back(eventTiming);
}

void Debugger::update(std::shared_ptr<Robot::CM730> cm730)
{
  auto const& cameraFrame = AgentState::get<CameraFrameState>();
  double seconds = Clock::getSeconds();

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

  auto eyeHsv = Colour::bgr2hsv(d_eyeColour);
  eyeHsv.v = fabs(sin(seconds*2)) * 255;
  auto eyeColour = Colour::hsv2bgr(eyeHsv);

  auto headHsv = Colour::bgr2hsv(d_headColour);
  headHsv.v = fabs(sin(seconds*3)) * 255;
  auto headColour = Colour::hsv2bgr(headHsv);

  int eyeInt =
     (eyeColour.r >> 3) |
    ((eyeColour.g >> 3) << 5) |
    ((eyeColour.b >> 3) << 10);

  if (eyeInt != d_lastEyeInt)
  {
    cm730->WriteWord(CM730::P_LED_EYE_L, eyeInt, 0);
    d_lastEyeInt = eyeInt;
  }

  int headInt =
     (headColour.r >> 3) |
    ((headColour.g >> 3) << 5) |
    ((headColour.b >> 3) << 10);

  if (headInt != d_lastHeadInt)
  {
    cm730->WriteWord(CM730::P_LED_HEAD_L, headInt, 0);
    d_lastHeadInt = headInt;
  }
}

void Debugger::showReady() { showHeadColour(Colour::bgr(255,0,0)); showEyeColour(Colour::bgr(255,0,0)); };

void Debugger::showSet() { showHeadColour(Colour::bgr(0,255,255)); showEyeColour(Colour::bgr(255,0,0)); };

void Debugger::showPlaying() { showHeadColour(Colour::bgr(0,255,0)); showEyeColour(Colour::bgr(255,0,0));}

void Debugger::showPenalized() { showHeadColour(Colour::bgr(0,0,255)); showEyeColour(Colour::bgr(255,0,0));}

void Debugger::showPaused() { showHeadColour(Colour::bgr(128,128,128)); showEyeColour(Colour::bgr(128,128,128)); }
