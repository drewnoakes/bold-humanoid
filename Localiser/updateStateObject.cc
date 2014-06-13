#include "localiser.ih"

void Localiser::updateStateObject()
{
  if (d_filterType == FilterType::Particle)
  {
    auto filter = static_pointer_cast<ParticleFilterUsed>(d_filter);
    MatrixXd states = filter->getParticles();
    MatrixXd weights = filter->getWeights();
    
    MatrixXd particles = MatrixXd::Ones(states.rows() + 1, weights.size());
    particles << states, weights.transpose();
    
    State::make<ParticleState>(particles, d_preNormWeightSum, d_uncertainty);
  }
}
