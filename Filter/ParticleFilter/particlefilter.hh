#ifndef BOLD_PARTICLEFILTER_HH
#define BOLD_PARTICLEFILTER_HH

#include <cassert>
#include <memory>
#include <vector>

#include "../filter.hh"
#include "../ParticleSamplerFactory/particlesamplerfactory.hh"

namespace bold
{
  template<int DIM>
  class ParticleFilter : public Filter<DIM>
  {
  public:
    typedef Eigen::Matrix<double,DIM,1> State;
    typedef std::pair<State,double> Particle;
    typedef std::function<State()> StateSampler;
    typedef std::function<std::shared_ptr<std::vector<Particle>>(std::shared_ptr<std::vector<Particle>>, unsigned)> ParticleResampler;

    ParticleFilter(unsigned initialSize,
                   StateSampler randomStateProvider,
                   ParticleResampler resampler)
    : d_particleCount(initialSize),
      d_randomStateProvider(randomStateProvider),
      d_particles(std::make_shared<std::vector<Particle>>(initialSize)),
      d_resampler(resampler)
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

    Particle extract() const override
    {
      // TODO implement
      return (*d_particles)[0]; // HACK to remove compiler warnings
    }

    std::shared_ptr<std::vector<Particle> const> getParticles() const { return d_particles; }

    void setParticleCount(int particleCount) { d_particleCount = particleCount; }

  private:
    unsigned d_particleCount;
    StateSampler d_randomStateProvider;
    std::shared_ptr<std::vector<Particle>> d_particles;
    ParticleResampler d_resampler;
  };
}

#endif
