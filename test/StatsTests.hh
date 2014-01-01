#include "gtest/gtest.h"

#include "helpers.hh"

#include "../stats/stats.hh"
#include "../stats/movingaverage.hh"

#include <Eigen/Core>

using namespace Eigen;
using namespace bold;

TEST (StatsTests, MovingAverage_double)
{
  MovingAverage<double> m(5);

  EXPECT_EQ( 1.0, m.next(1) );
  EXPECT_EQ( (1+2)/2.0, m.next(2) );
  EXPECT_EQ( (1+2+3)/3.0, m.next(3) );
  EXPECT_EQ( (1+2+3+4)/4.0, m.next(4) );
  EXPECT_EQ( (1+2+3+4+5)/5.0, m.next(5) );
  EXPECT_EQ( (2+3+4+5+6)/5.0, m.next(6) );
  EXPECT_EQ( (3+4+5+6+7)/5.0, m.next(7) );
  EXPECT_EQ( (4+5+6+7+8)/5.0, m.next(8) );
  EXPECT_EQ( (5+6+7+8+9)/5.0, m.next(9) );
}

TEST (StatsTests, MovingAverage_vector2d)
{
  MovingAverage<Vector2d> m(2);

  EXPECT_TRUE( VectorsEqual(Vector2d(1,1),     m.next(Vector2d(1,1))) );
  EXPECT_TRUE( VectorsEqual(Vector2d(1.5,1.5), m.next(Vector2d(2,2))) );
  EXPECT_TRUE( VectorsEqual(Vector2d(2.5,2.5), m.next(Vector2d(3,3))) );
  EXPECT_TRUE( VectorsEqual(Vector2d(3.5,3.5), m.next(Vector2d(4,4))) );
}

TEST (StatsTests, average_double)
{
  vector<double> doubles = {1, 2, 3, 4, 5, 6};
  EXPECT_EQ( (1+2+3+4+5+6)/6.0, bold::average<double>(doubles.begin(), doubles.end()) );
}

TEST (StatsTests, average_int)
{
  vector<int> ints = {1, 2, 3, 4, 5, 6};
  EXPECT_EQ( (1+2+3+4+5+6)/6, bold::average<int>(ints.begin(), ints.end()) );
}

TEST (StatsTests, varianceAndStdDev_double)
{
  vector<double> inputs = { 0.0188933, 0.08538519, 0.55895351, 0.51020354, 0.9623198 };
  vector<double> variances = { 0.057817, 0.045235, 0.041054 };

  EXPECT_NEAR( 0.057817, bold::variance<double>(inputs.begin() + 0, inputs.begin() + 3), 0.000001 );
  EXPECT_NEAR( 0.045235, bold::variance<double>(inputs.begin() + 1, inputs.begin() + 4), 0.000001 );
  EXPECT_NEAR( 0.041054, bold::variance<double>(inputs.begin() + 2, inputs.begin() + 5), 0.000001 );

  for (int i = 0; i < variances.size(); i++)
  {
    EXPECT_NEAR( variances[i],       bold::variance<double>(inputs.begin() + i, inputs.begin() + i + 3), 0.00001);
    EXPECT_NEAR( sqrt(variances[i]), bold::stdDev  <double>(inputs.begin() + i, inputs.begin() + i + 3), 0.00001);
  }
}

// TEST (StatsTests, stdDev_int)
// {
//   vector<int> ints = {456644, 5428, 39972, 3884, 7293, 348234, 42, 2335645};
//   EXPECT_EQ( 399642, bold::average<int>(ints.begin(), ints.end()) );
//   EXPECT_EQ( 802585, bold::stdDev<int>(ints.begin(), ints.end()) );
// }
