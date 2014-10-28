#include "lookatfeet.ih"

#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/BodyState/bodystate.hh"

vector<shared_ptr<Option>> LookAtFeet::runPolicy(Writer<StringBuffer>& writer)
{
  d_headModule->moveToDegs(d_panDegs->getValue(), d_tiltDegs->getValue());

  auto ballObs = State::get<AgentFrameState>()->getBallObservation();

  if (ballObs)
  {
    log::verbose("LookAtFeet::runPolicy") << "Ball pos in agent frame " << ballObs->transpose();
    d_avgBallPos.next(*ballObs);
    writer.String("ball");
    writer.StartArray();
    writer.Double(ballObs->x());
    writer.Double(ballObs->y());
    writer.EndArray(2);
  }
  else
  {
    writer.String("ball");
    writer.Null();
  }

  return {};
}
