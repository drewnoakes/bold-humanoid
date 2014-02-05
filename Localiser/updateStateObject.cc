#include "localiser.ih"

void Localiser::updateStateObject()
{
  MatrixXd states = d_filter->getParticles();
  MatrixXd weights = d_filter->getWeights();

  MatrixXd particles = MatrixXd::Ones(states.rows() + 1, weights.size());
  particles << states, weights.transpose();
  auto const& particleState = std::make_shared<ParticleState const>(particles, d_preNormWeightSum);

  AgentState::set(particleState);
}
