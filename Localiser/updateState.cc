#include "localiser.ih"

void Localiser::updateState()
{
  auto const& particleState = std::make_shared<ParticleState const>(d_filter->getParticles());

  AgentState::getInstance().set(particleState);
}