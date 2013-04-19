#ifndef BOLD_PARTICLEFILTER_HH
#define BOLD_PARTICLEFILTER_HH

#include <cassert>

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

    ParticleFilter(unsigned initialSize, double initialRandomizeRatio, StateSampler randomStateProvider, std::shared_ptr<ParticleSamplerFactory<DIM>> psf)
    : d_particleCount(initialSize),
      d_randomizeRatio(initialRandomizeRatio),
      d_randomStateProvider(randomStateProvider),
      d_particles(std::make_shared<std::vector<Particle>>(initialSize)),
      d_particleSamplerFactory(psf)
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
      auto newParticles = std::make_shared<std::vector<Particle>>();
      unsigned randomizeCount = d_particleCount * d_randomizeRatio;
      unsigned drawCount = d_particleCount - randomizeCount;

      assert(randomizeCount + drawCount == d_particleCount);

      for (int i = 0; i < randomizeCount; i++)
      {
        Particle p(d_randomStateProvider(), 0);
        newParticles->push_back(p);
      }

      for (int i = 0; i < drawCount; i++)
      {
        newParticles->push_back(drawSample());
      }

      assert(newParticles->size() == d_particleCount);

//       std::generate(newParticles->begin(), newParticles->end(), drawSample);

      d_particles = newParticles;
    }

    Particle extract() const override
    {
      // TODO implement
      return (*d_particles)[0]; // HACK to remove compiler warnings
    }

    std::shared_ptr<std::vector<Particle> const> getParticles() const { return d_particles; }

    void setParticleCount(int particleCount) { d_particleCount = particleCount; }
    void setRandomizeRatio(double ratio) { d_randomizeRatio = ratio; }

  private:
    unsigned d_particleCount;
    double d_randomizeRatio;
    StateSampler d_randomStateProvider;
    std::shared_ptr<std::vector<Particle>> d_particles;
    std::shared_ptr<ParticleSamplerFactory<DIM>> d_particleSamplerFactory;
  };
}

#endif
