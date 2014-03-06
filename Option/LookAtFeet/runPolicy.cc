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
    writer.String("ball").StartArray().Double(ballObs->x()).Double(ballObs->y()).EndArray(2);
  }
  else
  {
    writer.String("ball").Null();
  }

  return vector<shared_ptr<Option>>();
}
