#include "trackball.hh"

#include "../../Config/config.hh"
#include "../../Drawing/drawing.hh"
#include "../../FieldMap/fieldmap.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"

using namespace bold;
using namespace bold::Colour;
using namespace rapidjson;
using namespace std;

TrackBall::TrackBall(string id)
: Option(id, "TrackBall"),
  d_slowAverage(Config::getStaticValue<int>("options.track-ball.slow-window-size")),
  d_fastAverage(Config::getStaticValue<int>("options.track-ball.fast-window-size"))
{}

void TrackBall::reset()
{
  d_slowAverage.reset();
  d_fastAverage.reset();
}

vector<shared_ptr<Option>> TrackBall::runPolicy(Writer<StringBuffer>& writer)
{
  auto agentFrame = State::get<AgentFrameState>();

  ASSERT(agentFrame);

  auto ball = agentFrame->getBallObservation();

  if (!ball.hasValue())
  {
    reset();
    return {};
  }

  // Integrate the ball observation
  auto slow = d_slowAverage.next(ball->head<2>());
  auto fast = d_fastAverage.next(ball->head<2>());

  if (!d_slowAverage.isMature() || !d_fastAverage.isMature())
    return {};

  Draw::circle(Frame::Agent, slow, FieldMap::getBallRadius() * 0.5, bgr::orange, 0.4, 1);
  Draw::circle(Frame::Agent, fast, FieldMap::getBallRadius(),       bgr::orange, 0.8, 2);
  Draw::line  (Frame::Agent, slow, fast,                            bgr::orange, 0.8, 2);

  return {};
}
