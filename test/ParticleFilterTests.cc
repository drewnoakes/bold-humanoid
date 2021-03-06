#include <gtest/gtest.h>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <memory>
#include <stdexcept>

#include "../filters/Filter/ParticleFilter/particlefilter.hh"
#include "helpers.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (ParticleFilterTests, ESSCheck)
{
  // With uniform weights, ESS should be number of non-zero weights
  ESSFun<100> essFun;

  VectorXd weights(100);

  weights.fill(1.0 / 100);
  EXPECT_EQ(100, essFun.ESS(weights));

  weights.fill(0);
  weights.head<50>().fill(1.0 / 50);
  EXPECT_EQ(50, essFun.ESS(weights));
}

TEST (ParticleFilterTests, SystematicResample)
{
  constexpr int N = 6;
  SystematicResample<1, N> resample;
  MatrixXd particles = Matrix<double, 1, N>();
  for (unsigned i = 0; i < N; ++i)
    particles(i) = i;

  // With uniform weights, we should get the same samples
  VectorXd weights = Matrix<double, N, 1>::Constant(1.0 / N);
  auto newParticlesWeights = resample.resample(particles, weights, N);
  EXPECT_TRUE( MatricesEqual(particles, newParticlesWeights.first) );

  // Sampling half the samples should give either all odd or all even samples
  newParticlesWeights = resample.resample(particles, weights, N / 2);
  for (unsigned i = 0; i < N / 2; ++i)
    EXPECT_TRUE( newParticlesWeights.first(i) == particles(2 * i) || newParticlesWeights.first(i) == particles(2 * i + 1) );

  // A lone particle with non-zero weight should be sampled always
  weights.fill(0);
  weights(0) = 1;
  newParticlesWeights = resample.resample(particles, weights, N);
  for (unsigned i = 0 ; i < N; ++i)
    EXPECT_TRUE( VectorsEqual(VectorXd(newParticlesWeights.first.col(i)), VectorXd(particles.col(0))) );

  weights.fill(0);
  weights(N / 2) = 1;
  newParticlesWeights = resample.resample(particles, weights, N);
  for (unsigned i = 0 ; i < N; ++i)
    EXPECT_TRUE( VectorsEqual(VectorXd(newParticlesWeights.first.col(i)), VectorXd(particles.col(N / 2))) );

  weights.fill(0);
  weights(N - 1) = 1;
  newParticlesWeights = resample.resample(particles, weights, N);
  for (unsigned i = 0 ; i < N; ++i)
    EXPECT_TRUE( VectorsEqual(VectorXd(newParticlesWeights.first.col(i)), VectorXd(particles.col(N - 1))) );

  // A particle with twice the weight should be sampled twice as often
  // (in this test case where N is even, and weights are larger than 1.0 / N)
  weights.fill(0);
  weights(0) = 1.0 / 3.0;
  weights(1) = 2.0 / 3.0;
  newParticlesWeights = resample.resample(particles, weights, N);
  unsigned count1 = (newParticlesWeights.first.array() == 0).count();
  unsigned count2 = (newParticlesWeights.first.array() == 1).count();
  EXPECT_EQ( count2, 2 * count1 );
}
