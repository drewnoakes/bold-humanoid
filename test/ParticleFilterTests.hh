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

namespace bold
{
  template<int DIM>
  class ReversingSamplerFactory : public ParticleSamplerFactory<DIM>
  {
  public:
    ReversingSamplerFactory()
    : createCount(0),
      sampleCount(0)
    {}

    int createCount;
    int sampleCount;
    int index;

    typename Filter<DIM>::ParticleSampler create(std::shared_ptr<std::vector<typename Filter<DIM>::Particle>> const& particles) override
    {
      createCount++;
      index = particles->size() - 1;
      return [&]() {
        auto const& particle = (*particles)[index];
        this->index--;
        this->sampleCount++;
        return particle;
      };
    }
  };
}

TEST (ParticleFilterTests, basicOperation)
{
  auto samplingFactory = make_shared<ReversingSamplerFactory<2>>();

  EXPECT_EQ ( 0, samplingFactory->createCount );
  EXPECT_EQ ( 0, samplingFactory->sampleCount );

  int count = 10;
  unsigned i = 0;
  ParticleFilter<2> filter(count, [&](){ i++; return Vector2d(i, i); }, samplingFactory);

  EXPECT_EQ(count, i);

  i = 1;
  shared_ptr<vector<Filter<2>::Particle> const> particles = filter.getParticles();
  for (auto const& p : *particles)
  {
    EXPECT_TRUE( VectorsEqual(p.first, Vector2d(i, i)) );
    EXPECT_EQ( 0.0, p.second );
    i++;
  }

  EXPECT_EQ ( i, count + 1 );

  int predictCallCount = 0;
  filter.predict([&](Vector2d const& state) { predictCallCount++; return Vector2d(state.x() + 1, state.y() + 1); });

  EXPECT_EQ(count, predictCallCount);

  i = 1;
  for (auto const& p : *(filter.getParticles()))
  {
    EXPECT_TRUE( VectorsEqual(p.first, Vector2d(i + 1, i + 1)) );
    i++;
  }

  EXPECT_EQ ( i, count + 1 );

  // Set weights as [1,2,3,...]
  // The sampling function will return them in reverse order
  i = 0;
  filter.update([&i](Vector2d const& state) { i++; return i; });

  EXPECT_EQ ( 1, samplingFactory->createCount );
  EXPECT_EQ ( count, samplingFactory->sampleCount );

  i = count;
  for (auto const& p : *(filter.getParticles()))
  {
    EXPECT_EQ( double(i), p.second );
    i--;
  }
}
