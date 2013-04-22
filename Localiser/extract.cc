#include "localiser.ih"

ParticleFilter<3>::State Localiser::extract(shared_ptr<vector<Particle>> const& particles)
{
  assert(d_filter->getParticles()->size() != 0);

  // TODO HACK just taking the one with the highest weight for now

  auto bestParticle = (*d_filter->getParticles())[0];

  for (ParticleFilter<3>::Particle const& particle : *d_filter->getParticles())
  {
    if (particle.second > bestParticle.second)
    {
      bestParticle = particle;
    }
  }

  return bestParticle.first;
}
