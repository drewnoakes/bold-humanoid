#ifndef BOLD_LOCALISER_HH
#define BOLD_LOCALISER_HH

#include <memory>

#include "../AgentPosition/agentposition.hh"
#include "../AgentState/agentstate.hh"
#include "../Filter/ParticleFilter/particlefilter.hh"
#include "../ParticleSamplerFactory/WheelSamplerFactory/wheelsamplerfactory.hh"
#include "../StateObject/ParticleState/particlestate.hh"

namespace bold
{
  class Localiser
  {
  public:
    Localiser()
    : d_pos(-2.7, 0, 0.23, 0)
    {
      // TODO z value from body configuration (reuse code)
      auto samplerFactory = std::make_shared<WheelSamplerFactory<3>>();
      d_filter = std::make_shared<ParticleFilter<3>>(200, Localiser::randomState, samplerFactory);

      updateState();
    }

    static ParticleFilter<3>::State randomState()
    {
      // TODO generate an initial random state using config values
      // TODO can we use the game mode to bias the randomness? eg before kickoff, will be on a known side
    }

    void predict(/*motion data*/)
    {

    }

    void update()
    {
      // TODO implement
    }

    AgentPosition position() const { return d_pos; }

  private:
    void updateState()
    {
      auto const& particleState = std::make_shared<ParticleState const>(d_filter->getParticles());

      AgentState::getInstance().set(particleState);
    }

    AgentPosition d_pos;
    std::shared_ptr<ParticleFilter<3>> d_filter;
  };
}

#endif