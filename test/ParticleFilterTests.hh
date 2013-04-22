#include "gtest/gtest.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <memory>
#include <stdexcept>

#include "helpers.hh"
#include "../Filter/ParticleFilter/particlefilter.hh"
#include "../ParticleSamplerFactory/WheelSamplerFactory/wheelsamplerfactory.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (ParticleFilterTests, basicOperation)
{
  unsigned resampleCallCount = 0;
  ParticleFilter<2>::ParticleResampler resampler
   = [&resampleCallCount](shared_ptr<vector<ParticleFilter<2>::Particle>> particles, unsigned particleCount)
    {
      resampleCallCount++;
      auto newParticles = make_shared<vector<ParticleFilter<2>::Particle>>();
      for (int i = particleCount - 1; i >= 0; i--)
      {
        newParticles->push_back((*particles)[i]);
      }
      return newParticles;
    };

  ParticleFilter<2>::ParticleExtractor extractor =
    [this](shared_ptr<vector<ParticleFilter<2>::Particle>> particles) { return (*particles)[0].first; };

  EXPECT_EQ ( 0, resampleCallCount );

  int particleCount = 10;
  int i = 0;
  ParticleFilter<2> filter(particleCount, [&](){ i++; return Vector2d(i, i); }, resampler, extractor);

  EXPECT_EQ(particleCount, i);

  i = 1;
  shared_ptr<vector<ParticleFilter<2>::Particle> const> particles = filter.getParticles();
  for (auto const& p : *particles)
  {
    EXPECT_TRUE( VectorsEqual(p.first, Vector2d(i, i)) );
    EXPECT_EQ( 0.0, p.second );
    i++;
  }

  EXPECT_EQ ( i, particleCount + 1 );

  int predictCallCount = 0;
  filter.predict([&](Vector2d const& state) { predictCallCount++; return Vector2d(state.x() + 1, state.y() + 1); });

  EXPECT_EQ(particleCount, predictCallCount);

  i = 1;
  for (auto const& p : *(filter.getParticles()))
  {
    EXPECT_TRUE( VectorsEqual(p.first, Vector2d(i + 1, i + 1)) );
    i++;
  }

  EXPECT_EQ ( i, particleCount + 1 );

  // Set weights as [1,2,3,...]
  // The sampling function will return them in reverse order
  i = 0;
  filter.update([&i](Vector2d const& state) { i++; return i; });

  EXPECT_EQ ( 1, resampleCallCount );

  i = particleCount;
  for (auto const& p : *(filter.getParticles()))
  {
    EXPECT_EQ( double(i), p.second );
    i--;
  }
}
