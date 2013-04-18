#ifndef BOLD_PARTICLESAMPLERFACTORY_HH
#define BOLD_PARTICLESAMPLERFACTORY_HH

#include <vector>
#include <memory>

#include "../Filter/filter.hh"

namespace bold
{
  template<int DIM>
  class ParticleSamplerFactory
  {
  public:
    virtual typename Filter<DIM>::ParticleSampler create(std::shared_ptr<std::vector<typename Filter<DIM>::Particle>> const& particles) = 0;
  };
}

#endif