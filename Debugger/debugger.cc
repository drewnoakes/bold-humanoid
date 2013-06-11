#include "debugger.ih"

#define LED_RED   (0x01);
#define LED_BLUE  (0x02);
#define LED_GREEN (0x04);

Debugger::Debugger()
: d_lastLedFlags(0xff),
  d_lastEyeInt(0),
  d_lastHeadInt(0),
  d_gameControllerMessageCount(0),
  d_ignoredMessageCount(0),
  d_eyeColour(0,0,0),
  d_headColour(0,0,0)
{}

void Debugger::update(shared_ptr<CM730> cm730)
{
  //
  // Update Hardware LEDs
  //

  auto const& cameraFrame = AgentState::get<CameraFrameState>();
  double seconds = Clock::getSeconds();

  if (!cameraFrame)
    return;

  int ledFlags = 0;
  if (cameraFrame->getBallObservation().hasValue())
    ledFlags |= LED_RED;

  if (d_gameControllerMessageCount != 0)
    ledFlags |= LED_BLUE;

  if (cameraFrame->getGoalObservations().size() > 1)
    ledFlags |= LED_GREEN;

  if (ledFlags != d_lastLedFlags)
  {
    // the value changed, so write it
    cm730->writeByte(CM730::ID_CM, CM730::P_LED_PANEL, ledFlags, NULL);
    d_lastLedFlags = ledFlags;
  }

  auto setColor = [&cm730](Colour::bgr const& bgr, int* lastInt, int const& targetId, double const& v)
  {
    auto hsv = Colour::bgr2hsv(bgr);
    hsv.v = v;

    auto color = Colour::hsv2bgr(hsv);

    int intValue =
       (color.r >> 3) |
      ((color.g >> 3) << 5) |
      ((color.b >> 3) << 10);

    if (intValue != (*lastInt))
    {
      cm730->writeWord(CM730::ID_CM, targetId, intValue, 0);
      *lastInt = intValue;
    }
  };

  setColor(d_eyeColour,  &d_lastEyeInt,  CM730::P_LED_EYE_L,  fabs(sin(seconds*2)) * 255);
  setColor(d_headColour, &d_lastHeadInt, CM730::P_LED_HEAD_L, fabs(sin(seconds*3)) * 255);

  //
  // Update DebugState object
  //

  bool hasRed = d_lastLedFlags & LED_RED;
  bool hasGreen = d_lastLedFlags & LED_RED;
  bool hasBlue = d_lastLedFlags & LED_RED;
  AgentState::getInstance().set(
    make_shared<DebugState const>(
      d_gameControllerMessageCount, d_ignoredMessageCount,
      d_eyeColour, d_headColour, 
      hasRed, hasGreen, hasBlue
    )
  );

  // clear accumulators for next cycle
  d_gameControllerMessageCount = 0;
  d_ignoredMessageCount = 0;
}

void Debugger::showReady() { showHeadColour(Colour::bgr(255,0,0)); showEyeColour(Colour::bgr(255,0,0)); };

void Debugger::showSet() { showHeadColour(Colour::bgr(0,255,255)); showEyeColour(Colour::bgr(255,0,0)); };

void Debugger::showPlaying() { showHeadColour(Colour::bgr(0,255,0)); showEyeColour(Colour::bgr(255,0,0)); }

void Debugger::showPenalized() { showHeadColour(Colour::bgr(0,0,255)); showEyeColour(Colour::bgr(255,0,0)); }

void Debugger::showPaused() { showHeadColour(Colour::bgr(128,128,128)); showEyeColour(Colour::bgr(128,128,128)); }
