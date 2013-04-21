#include "localiser.ih"

ParticleFilter<3>::State Localiser::createRandomState()
{
  // TODO can we use the game mode to bias the randomness? eg before kickoff, will be on a known side, or nearer prior known locations
  return ParticleFilter<3>::State(d_fieldXRng(), d_fieldYRng(), d_thetaRng());
}