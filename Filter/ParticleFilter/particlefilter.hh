#ifndef BOLD_PARTICLEFILTER_HH
#define BOLD_PARTICLEFILTER_HH

#include "../filter.hh"
#include "../ParticleSamplerFactory/particlesamplerfactory.hh"

#include <memory>

namespace bold
{
  template<int DIM>
  class ParticleFilter : public Filter<DIM>
  {
  public:
    typedef Eigen::Matrix<double,DIM,1> State;
    typedef std::pair<State,double> Particle;
    typedef std::function<State()> StateSampler;
    typedef std::function<Particle()> ParticleSampler;

    ParticleFilter(int initialSize, StateSampler randomStateProvider, std::shared_ptr<ParticleSamplerFactory<DIM>> psf)
    : d_particles(std::make_shared<std::vector<Particle>>(initialSize)),
      d_particleSamplerFactory(psf),
      d_randomStateProvider(randomStateProvider)
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
      ParticleSampler drawSample = d_particleSamplerFactory->create(d_particles);
      auto newParticles = std::make_shared<std::vector<Particle>>(d_particles->size());
      std::generate(newParticles->begin(), newParticles->end(), drawSample);

      d_particles = newParticles;
    }

    Particle extract() const override
    {
      // TODO implement
      return (*d_particles)[0]; // HACK to remove compiler warnings
    }

    std::shared_ptr<std::vector<Particle> const> getParticles() const { return d_particles; }

  private:
    StateSampler d_randomStateProvider;
    std::shared_ptr<std::vector<Particle>> d_particles;
    std::shared_ptr<ParticleSamplerFactory<DIM>> d_particleSamplerFactory;
  };
}

#endif
