#include "localiser.ih"

shared_ptr<vector<Particle>> Localiser::resample(shared_ptr<vector<Particle>> const& particles, unsigned particleCount)
{
  auto newParticles = make_shared<vector<Particle>>();

  auto cameraState = AgentState::get<CameraFrameState>();

  bool seenLandmark = cameraState->getGoalObservations().size() >= d_minGoalsNeeded || (d_useLines && cameraState->getObservedLineSegments().size() != 0);

  unsigned randomizeCount = seenLandmark
    ? particleCount * d_randomizeRatio
    : 0; // don't randomize if we haven't seen any fixed landmarks this cycle

  unsigned drawCount = particleCount - randomizeCount;

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

  assert(newParticles->size() == particleCount);

  return newParticles;
}