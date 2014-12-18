#include "handlerhelper.hh"

#include "../../Voice/voice.hh"
#include "../../BehaviourControl/behaviourcontrol.hh"

using namespace bold;
using namespace std;

HandlerHelper::HandlerHelper(shared_ptr<Voice> voice, shared_ptr<BehaviourControl> behaviourControl)
: TypedStateObserver<GameState>("Handler Helper", ThreadId::ThinkLoop),
  d_voice(voice),
  d_behaviourControl(behaviourControl),
  d_lastPenaltyLiftAnnounceTime(0)
{
  ASSERT(d_voice);
}

void HandlerHelper::observeTyped(shared_ptr<GameState const> const& state, SequentialTimer& timer)
{
  // State may be null if the game controller has been lost for a while and we decide to forget its prior state
  if (!state)
    return;

  // Announce when our penalty period is over, in case the assistant referee has failed to unpenalise us
  auto const& myPlayerInfo = state->getMyPlayerInfo();
  if (myPlayerInfo.hasPenalty() && myPlayerInfo.getPenaltyType() != PenaltyType::SUBSTITUTE)
  {
    if (myPlayerInfo.getSecondsUntilPenaltyLifted() == 0)
    {
      if (Clock::getSecondsSince(d_lastPenaltyLiftAnnounceTime) > 10)
      {
        d_voice->say("Penalty complete");
        d_lastPenaltyLiftAnnounceTime = Clock::getTimestamp();
      }
    }
  }

  // Announce when game play mode is initial/ready/set and robot is paused
  if (d_behaviourControl->getPlayerStatus() == PlayerStatus::Paused)
  {
    switch (state->getPlayMode())
    {
      case PlayMode::INITIAL:
      case PlayMode::READY:
      case PlayMode::SET:
        if (Clock::getSecondsSince(d_lastPauseAtGameStartTime) > 10)
        {
          d_voice->say("I am paused and the game is starting");
          d_lastPauseAtGameStartTime = Clock::getTimestamp();
        }
        break;
      default:
        break;
    }
  }
}
