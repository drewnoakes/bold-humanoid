#ifndef BOLD_WHEELSAMPLERFACTORY_HH
#define BOLD_WHEELSAMPLERFACTORY_HH

#include <iterator>
#include <random>
#include <vector>

#include "../../Filter/filter.hh"
#include "../../Filter/ParticleFilter/particlefilter.hh"
#include "../ParticleSamplerFactory/particlesamplerfactory.hh"

namespace bold
{
  template<int DIM>
  class WheelSamplerFactory : public ParticleSamplerFactory<DIM>
  {
  public:
    WheelSamplerFactory()
    {
      std::default_random_engine generator;
      std::uniform_real_distribution<double> distribution(0,1);
      d_rnd = std::bind(distribution, generator);
    }

    typename Filter<DIM>::ParticleSampler create(std::shared_ptr<std::vector<typename Filter<DIM>::Particle>> const& particles) override
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

#endif
