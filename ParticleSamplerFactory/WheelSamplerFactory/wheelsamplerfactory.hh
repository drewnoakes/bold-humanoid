#pragma once

#include <iterator>
#include <vector>
#include <functional>

#include "../../Filter/ParticleFilter/particlefilter.hh"
#include "../../Math/math.hh"
#include "../ParticleSamplerFactory/particlesamplerfactory.hh"

namespace bold
{
  template<int DIM>
  class WheelSamplerFactory : public ParticleSamplerFactory<DIM>
  {
  public:
    WheelSamplerFactory()
    {
      d_rnd = Math::createUniformRng(0, 1);
    }

    typename ParticleFilter<DIM>::ParticleSampler create(std::shared_ptr<std::vector<typename ParticleFilter<DIM>::Particle>> const& particles) override
    {
      d_index = d_rnd() * particles->size();
      d_beta = 0.0;

      d_maxWeight = std::max_element(
        particles->begin(), particles->end(),
        [](typename ParticleFilter<DIM>::Particle const& p1, typename ParticleFilter<DIM>::Particle const& p2) {
          return p1.second < p2.second;
        })->second;

      return [&]() {
        d_beta += d_rnd() * 2 * d_maxWeight;
        double weight = (*particles)[d_index].second;
        while (d_beta > weight) {
          d_beta -= weight;
          d_index = (d_index + 1) % particles->size();
          weight = (*particles)[d_index].second;
        }
        return (*particles)[d_index];
      };
    }

  private:
    int d_index;
    double d_beta;
    double d_maxWeight;
    std::function<double()> d_rnd;
  };
}
