#pragma once

#include <vector>
#include <memory>

#include "../Filter/ParticleFilter/particlefilter.hh"

namespace bold
{
  template<int DIM>
  class ParticleSamplerFactory
  {
  public:
    virtual typename ParticleFilter<DIM>::ParticleSampler create(std::shared_ptr<std::vector<typename bold::ParticleFilter<DIM>::Particle>> const& particles) = 0;
  };
}
