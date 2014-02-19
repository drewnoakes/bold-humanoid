#include "lookatfeet.ih"

#include "../../StateObject/AgentFrameState/agentframestate.hh"

std::vector<std::shared_ptr<Option>> LookAtFeet::runPolicy()
{
  d_headModule->moveToDegs(d_panDegs->getValue(), d_tiltDegs->getValue());

  auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();

  if (ballObs)
    log::verbose("LookAtFeet::runPolicy") << "Ball pos in agent frame " << ballObs->transpose();

  return std::vector<std::shared_ptr<Option>>();
}
