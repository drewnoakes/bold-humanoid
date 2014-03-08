#include "vocaliser.hh"

#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

#include <sstream>

using namespace bold;
using namespace std;

Vocaliser::Vocaliser(shared_ptr<Voice> voice)
: TypedStateObserver<AgentFrameState>("Vocaliser", ThreadId::ThinkLoop),
  d_enableBallPos(Config::getSetting<bool>("vocaliser.announce-ball-position")),
  d_voice(voice)
{}

void Vocaliser::observeTyped(std::shared_ptr<AgentFrameState const> const& agentFrameState, SequentialTimer& timer)
{
  if (d_enableBallPos->getValue())
  {
    if (int(fmod(agentFrameState->getThinkCycleNumber(), 4*30)) == 0 && d_voice->queueLength() == 0)
    {
      if (agentFrameState->isBallVisible())
      {
        auto const& ballPos = agentFrameState->getBallObservation();

        stringstream s;
        s << int(fabs(ballPos->x()*100)) << (ballPos->x() > 0 ? "right " : "left ");
        s << int(fabs(ballPos->y()*100)) << (ballPos->y() > 0 ? "forward" : "backward");

        d_voice->say(SpeechTask({s.str(), 180, true}));
      }
      else
      {
        d_voice->say("no ball");
      }
    }
  }
}
