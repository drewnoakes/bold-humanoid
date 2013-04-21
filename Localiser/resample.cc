#include "localiser.ih"

shared_ptr<vector<Particle>> Localiser::resample(shared_ptr<vector<Particle>> const& particles, unsigned particleCount)
{
  auto newParticles = make_shared<vector<Particle>>();

  unsigned randomizeCount = particleCount * d_randomizeRatio;
  unsigned drawCount = particleCount - randomizeCount;

  assert(randomizeCount + drawCount == particleCount);

  for (int i = 0; i < randomizeCount; i++)
  {
    Particle p(createRandomState(), 0);
    newParticles->push_back(p);
  }

  auto drawSample = d_wsf.create(particles);

  for (int i = 0; i < drawCount; i++)
  {
    // TODO apply noise to the generated samples? probably do that in the motion model, even if no motion
    newParticles->push_back(drawSample());
  }

  return newParticles;
}