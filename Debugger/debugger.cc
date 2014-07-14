#include "debugger.hh"

#include "../Agent/agent.hh"
#include "../BehaviourControl/behaviourcontrol.hh"
#include "../State/state.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"
#include "../StateObject/DebugState/debugstate.hh"
#include "../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../StateObserver/ButtonObserver/buttonobserver.hh"
#include "../stats/movingaverage.hh"
#include "../Voice/voice.hh"

using namespace bold;
using namespace bold::Colour;
using namespace robocup;
using namespace std;

Debugger::Debugger(
  Agent* agent,
  shared_ptr<BehaviourControl> behaviourControl, shared_ptr<DebugControl> debugControl,
  shared_ptr<Voice> voice, shared_ptr<ButtonObserver> buttonObserver)
: d_agent(agent),
  d_behaviourControl(behaviourControl),
  d_debugControl(debugControl),
  d_voice(voice),
  d_leftButtonTracker(buttonObserver->track(Button::Left)),
  d_showDazzle(false),
  d_gameControllerMessageCount(0),
  d_ignoredMessageCount(0),
  d_sentTeamMessageCount(0),
  d_receivedTeamMessageCount(0),
  d_sentDrawbridgeMessageCount(0)
{}

void Debugger::update()
{
  //
  // Update Hardware LEDs
  //

  auto const& stationaryMap = State::get<StationaryMapState>();
  auto const& cameraFrame = State::get<CameraFrameState>();

  if (stationaryMap)
  {
    d_debugControl->setPanelLedStates(
      /*red  */ stationaryMap->hasEnoughBallObservations(),
      /*blue */ d_gameControllerMessageCount != 0,
      /*green*/ stationaryMap->getSatisfactoryGoalPostCount() != 0
    );
  }
  else if (cameraFrame)
  {
    d_debugControl->setPanelLedStates(
      /*red  */ cameraFrame->getBallObservation().hasValue(),
      /*blue */ d_gameControllerMessageCount != 0,
      /*green*/ cameraFrame->getGoalObservations().size() != 0
    );
  }
  else
  {
    d_debugControl->setPanelLedStates(false, false, false);
  }

  //
  // Eye and forehead colours
  //

  bgr eyeColour = bgr::orange;
  bgr foreheadColour = bgr::orange;

  if (d_agent->isStopRequested())
  {
    eyeColour = bgr::black;
    foreheadColour = bgr::black;
  }
  else if (d_showDazzle)
  {
    static auto prng = Math::createUniformRng(0, 1);
    static MovingAverage<double> smoothedHue(3);
    static MovingAverage<double> smoothedValue(3);

    double hue = smoothedHue.next(prng());
    double value = smoothedValue.next(prng());

    bgr colour = hsv(int(hue * 255), 255, int(value * 255)).toBgr();

    foreheadColour = colour;
    eyeColour = colour;
  }
  else
  {
    // Set forehead colour
    if (d_behaviourControl->getPlayerStatus() == PlayerStatus::Paused)
    {
      foreheadColour = bgr::grey;
    }
    else if (d_behaviourControl->getPlayerStatus() == PlayerStatus::Penalised)
    {
      foreheadColour = bgr::lightRed;
    }
    else
    {
      switch (d_behaviourControl->getPlayMode())
      {
        case PlayMode::INITIAL:  foreheadColour = bgr::darkBlue;   break;
        case PlayMode::READY:    foreheadColour = bgr::lightBlue;  break;
        case PlayMode::SET:      foreheadColour = bgr::yellow;     break;
        case PlayMode::PLAYING:  foreheadColour = bgr::lightGreen; break;
        case PlayMode::FINISHED: foreheadColour = bgr(30,30,30);   break;
      }
    }

    // Set eye colour
    switch (d_behaviourControl->getPlayerRole())
    {
      case PlayerRole::Idle:           eyeColour = bgr(64,0,0);    break;
      case PlayerRole::Defender:       eyeColour = bgr(200,0,0);   break;
      case PlayerRole::Supporter:      eyeColour = bgr(0,200,0);   break;
      case PlayerRole::Striker:
      case PlayerRole::PenaltyStriker: eyeColour = bgr(148,0,211); break;
      case PlayerRole::Keeper:
      case PlayerRole::PenaltyKeeper:  eyeColour = bgr(64,64,64);  break;
      case PlayerRole::Other:
      default:                         eyeColour = bgr(139,0,139); break;
    }
  }

  auto modulateColor = [](Colour::bgr const& bgr, uchar const& v)
  {
    auto hsv = Colour::bgr2hsv(bgr);
    hsv.v = v;
    return hsv2bgr(hsv);
  };

  double seconds = Clock::getSeconds();
  d_debugControl->setEyeColour(modulateColor(eyeColour, fabs(sin(seconds*2)) * 255));
  d_debugControl->setForeheadColour(modulateColor(foreheadColour, fabs(sin(seconds*3)) * 255));

  //
  // Update DebugState object
  //

  State::make<DebugState>(
    d_gameControllerMessageCount, d_ignoredMessageCount,
    d_sentTeamMessageCount, d_receivedTeamMessageCount,
    d_sentDrawbridgeMessageCount,
    d_debugControl
  );

  // clear accumulators for next cycle
  d_gameControllerMessageCount = 0;
  d_ignoredMessageCount = 0;
  d_sentTeamMessageCount = 0;
  d_receivedTeamMessageCount = 0;
  d_sentDrawbridgeMessageCount = 0;

  // Look for left button presses while paused
  if (d_leftButtonTracker->isPressedForMillis(20) && d_behaviourControl->getPlayerStatus() == PlayerStatus::Paused)
  {
    auto hw = State::get<HardwareState>();
    auto team = State::get<TeamState>();

    stringstream msg;
    msg << hw->getCM730State().voltage << " volts.";

    // TODO need to ensure old teammate data is removed from TeamState
    if (team && team->players().size() > 1)
    {
      msg << " I hear team mates ";
      for (auto const& mate : team->players())
      {
        if (!mate.isMe())
          msg << mate.uniformNumber << ", ";
      }
      msg << ".";
    }
    else
    {
      msg << " No team mates.";
    }

    d_voice->say(msg.str());
  }
}
