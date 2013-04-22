#include "math.ih"

function<double()> Math::createUniformRng(double min, double max, bool randomSeed)
{
  uniform_real_distribution<double> distribution(min, max);

  if (randomSeed)
  {
    auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    return bind(distribution, default_random_engine(seed));
  }

  return bind(distribution, default_random_engine());
}

function<double()> Math::createNormalRng(double mean, double stddev, bool randomSeed)
{
  normal_distribution<double> distribution(mean, stddev);

  if (randomSeed)
  {
    auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    return bind(distribution, default_random_engine(seed));
  }

  return bind(distribution, default_random_engine());
}
