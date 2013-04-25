#include "gtest/gtest.h"

#include "../MovingAverage/movingaverage.hh"
#include "helpers.hh"

#include <Eigen/Core>

using namespace Eigen;
using namespace bold;

TEST (MovingAverageTests, basics)
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