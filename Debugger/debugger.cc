#include "debugger.ih"

#include "../BodyControl/bodycontrol.hh"
#include "../util/log.hh"

using namespace bold::Colour;

Debugger::Debugger(std::shared_ptr<DebugControl> debugControl)
: d_debugControl(debugControl),
  d_gameControllerMessageCount(0),
  d_ignoredMessageCount(0),
  d_eyeColour(0,0,0),
  d_foreheadColour(0,0,0)
{}

void Debugger::update()
{
  //
  // Update Hardware LEDs
  //

  auto const& cameraFrame = State::get<CameraFrameState>();

  if (!cameraFrame)
    return;

  d_debugControl->setPanelLedStates(
    /*red  */ cameraFrame->getBallObservation().hasValue(),
    /*blue */ d_gameControllerMessageCount != 0,
    /*green*/ cameraFrame->getGoalObservations().size() > 1
  );

  auto modulateColor = [](Colour::bgr const& bgr, uchar const& v)
  {
    auto hsv = Colour::bgr2hsv(bgr);
    hsv.v = v;
    return Colour::hsv2bgr(hsv);
  };

  double seconds = Clock::getSeconds();
  d_debugControl->setEyeColour(modulateColor(d_eyeColour, fabs(sin(seconds*2)) * 255));
  d_debugControl->setForeheadColour(modulateColor(d_foreheadColour, fabs(sin(seconds*3)) * 255));

  //
  // Update DebugState object
  //

  // TODO track whether there's actually anything to update, avoiding unnecessary DebugState changes

  State::make<DebugState>(
    d_gameControllerMessageCount, d_ignoredMessageCount,
    d_sentTeamMessageCount, d_receivedTeamMessageCount,
    d_sentDrawbridgeMessageCount,
    d_debugControl
  );

  // clear accumulators for next cycle
  d_gameControllerMessageCount = 0;
  d_ignoredMessageCount = 0;
}

void Debugger::notifyReceivedGameControllerMessage()
{
  d_gameControllerMessageCount++;
}

void Debugger::showReady()       { d_foreheadColour = bgr(255,0,0);     /* d_eyeColour = bgr(255,0,0);     */ }
void Debugger::showSet()         { d_foreheadColour = bgr(0,255,255);   /* d_eyeColour = bgr(255,0,0);     */ }
void Debugger::showPlaying()     { d_foreheadColour = bgr(0,255,0);     /* d_eyeColour = bgr(64,64,64);    */ }
void Debugger::showPenalized()   { d_foreheadColour = bgr(0,0,255);     /* d_eyeColour = bgr(255,0,0);     */ }
void Debugger::showPaused()      { d_foreheadColour = bgr(128,128,128); /* d_eyeColour = bgr(128,128,128); */ }
void Debugger::showExitedAgent() { d_foreheadColour = bgr(0,0,0);       /* d_eyeColour = bgr(0,0,0);       */ }

void Debugger::showRole(PlayerRole role)
{
  switch (role)
  {
    case PlayerRole::Idle:           d_eyeColour = bgr(64,0,0); break;
    case PlayerRole::Defender:       d_eyeColour = bgr(200,0,0); break;
    case PlayerRole::Supporter:      d_eyeColour = bgr(0,200,0); break;
    case PlayerRole::Striker:
    case PlayerRole::PenaltyStriker: d_eyeColour = bgr(148,0,211); break;
    case PlayerRole::Keeper:
    case PlayerRole::PenaltyKeeper:  d_eyeColour = bgr(64,64,64); break;
    case PlayerRole::Other:
    default:                         d_eyeColour = bgr(139,0,139); break;
  }
}

void Debugger::showExitingAgent()
{
  static auto prng = Math::createUniformRng(0, 1);
  static MovingAverage<double> smoothedHue(3);
  static MovingAverage<double> smoothedValue(3);

  double hue = smoothedHue.next(prng());
  double value = smoothedValue.next(prng());

  Colour::bgr colour = Colour::hsv(int(hue * 255), 255, int(value * 255)).toBgr();

  d_foreheadColour = colour;
  d_eyeColour = colour;
}
