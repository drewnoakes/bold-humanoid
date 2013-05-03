#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include "../filter.hh"

namespace bold
{
  template<int DIM>
  class ParticleFilter : public Filter<DIM>
  {
  public:
    typedef Eigen::Matrix<double,DIM,1> State;
    typedef std::pair<State,double> Particle;
    typedef std::function<Particle()> ParticleSampler;
    typedef std::function<State()> StateSampler;
    typedef std::function<std::shared_ptr<std::vector<Particle>>(std::shared_ptr<std::vector<Particle>>, unsigned)> ParticleResampler;
    typedef std::function<State(std::shared_ptr<std::vector<Particle>>)> ParticleExtractor;

    ParticleFilter(unsigned initialSize,
                   StateSampler randomStateProvider,
                   ParticleResampler resampler,
                   ParticleExtractor extractor)
    : d_particleCount(initialSize),
      d_randomStateProvider(randomStateProvider),
      d_particles(std::make_shared<std::vector<Particle>>(initialSize)),
      d_resampler(resampler),
      d_extractor(extractor)
    {
      randomise();
    }

    void randomise()
    {
      std::generate(d_particles->begin(), d_particles->end(),
        [&]()
        {
          return Particle(d_randomStateProvider(), 0);
        });
    }

    void predict(std::function<State(State const&)> motionModel) override
    {
      // Update particles in-place
      std::transform(d_particles->begin(), d_particles->end(),
         d_particles->begin(),
         [&](Particle const& particle)
         {
           return Particle(motionModel(particle.first), particle.second);
         });
    }

    void update(std::function<double(State const&)> observationModel) override
    {
      // Calculate the probability of each particle given the observation data
      for (auto& particle : *d_particles)
      {
        // Assign weight
        particle.second = observationModel(particle.first);
      }

      // Build the next generation
      d_particles = d_resampler(d_particles, d_particleCount);
      assert(d_particles->size() == d_particleCount);
    }

    State extract() const override
    {
      return d_extractor(d_particles);
    }

    // TODO does returning this as 'const' mean that the data is copied? if so, remove const

    std::shared_ptr<std::vector<Particle> const> getParticles() const { return d_particles; }

    void setParticleCount(unsigned particleCount)
    {
      assert(particleCount != 0);
      d_particleCount = particleCount;
    }

  private:
    unsigned d_particleCount;
    StateSampler d_randomStateProvider;
    std::shared_ptr<std::vector<Particle>> d_particles;
    ParticleResampler d_resampler;
    ParticleExtractor d_extractor;
  };
}
