#include "gtest/gtest.h"

#include "helpers.hh"

#include "../stats/movingaverage.hh"

#include <Eigen/Core>

using namespace Eigen;
using namespace bold;

TEST (MovingAverageTests, double)
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

TEST (MovingAverageTests, vector2d)
{
  MovingAverage<Vector2d> m(2);

  EXPECT_TRUE( VectorsEqual(Vector2d(1,1),     m.next(Vector2d(1,1))) );
  EXPECT_TRUE( VectorsEqual(Vector2d(1.5,1.5), m.next(Vector2d(2,2))) );
  EXPECT_TRUE( VectorsEqual(Vector2d(2.5,2.5), m.next(Vector2d(3,3))) );
  EXPECT_TRUE( VectorsEqual(Vector2d(3.5,3.5), m.next(Vector2d(4,4))) );
}
