#include "debugger.ih"

#include "../BodyControl/bodycontrol.hh"
#include "../util/log.hh"

Debugger::Debugger(std::shared_ptr<DebugControl> debugControl)
: d_debugControl(debugControl),
  d_gameControllerMessageCount(0),
  d_ignoredMessageCount(0),
  d_seenGameControllerMessageYet(false),
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

  State::set(
    make_shared<DebugState const>(
      d_gameControllerMessageCount, d_ignoredMessageCount,
      d_sentTeamMessageCount, d_receivedTeamMessageCount,
      d_debugControl
    )
  );

  // clear accumulators for next cycle
  d_gameControllerMessageCount = 0;
  d_ignoredMessageCount = 0;
}

void Debugger::notifyReceivedGameControllerMessage()
{
  if (!d_seenGameControllerMessageYet)
  {
    log::info("Debugger::notifyReceivedGameControllerMessage") << "Seen first message from game controller";
    d_seenGameControllerMessageYet = true;
  }

  d_gameControllerMessageCount++;
}

void Debugger::showReady()       { d_foreheadColour = Colour::bgr(255,0,0);     d_eyeColour = Colour::bgr(255,0,0); };
void Debugger::showSet()         { d_foreheadColour = Colour::bgr(0,255,255);   d_eyeColour = Colour::bgr(255,0,0); };
void Debugger::showPlaying()     { d_foreheadColour = Colour::bgr(0,255,0);     d_eyeColour = Colour::bgr(255,0,0); }
void Debugger::showPenalized()   { d_foreheadColour = Colour::bgr(0,0,255);     d_eyeColour = Colour::bgr(255,0,0); }
void Debugger::showPaused()      { d_foreheadColour = Colour::bgr(128,128,128); d_eyeColour = Colour::bgr(128,128,128); }
void Debugger::showExitedAgent() { d_foreheadColour = Colour::bgr(0,0,0);       d_eyeColour = Colour::bgr(0,0,0); }

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
