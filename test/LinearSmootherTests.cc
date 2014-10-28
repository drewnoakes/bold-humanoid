#include <gtest/gtest.h>

#include "../Smoother/LinearSmoother/linearsmoother.hh"

using namespace std;
using namespace bold;

TEST (LinearSmootherTests, basics)
{
  LinearSmoother s(0, 5);

  EXPECT_EQ ( 0, s.getCurrent() );
  EXPECT_EQ ( 0, s.getTarget() );
  EXPECT_EQ ( 0, s.getNext() );
  EXPECT_EQ ( 0, s.getNext() );
  EXPECT_EQ ( 0, s.getNext() );

  s.setTarget(22);

  EXPECT_EQ ( 5, s.getNext() );
  EXPECT_EQ ( 10, s.getNext() );
  EXPECT_EQ ( 15, s.getNext() );
  EXPECT_EQ ( 20, s.getNext() );
  EXPECT_EQ ( 22, s.getNext() );
  EXPECT_EQ ( 22, s.getNext() );
  EXPECT_EQ ( 22, s.getNext() );

  s.setTarget(-22);

  EXPECT_EQ ( 17, s.getNext() );
  EXPECT_EQ ( 12, s.getNext() );
  EXPECT_EQ ( 7, s.getNext() );
  EXPECT_EQ ( 2, s.getNext() );
  EXPECT_EQ ( -3, s.getNext() );
  EXPECT_EQ ( -8, s.getNext() );
  EXPECT_EQ ( -13, s.getNext() );
  EXPECT_EQ ( -18, s.getNext() );
  EXPECT_EQ ( -22, s.getNext() );
  EXPECT_EQ ( -22, s.getNext() );
  EXPECT_EQ ( -22, s.getNext() );

  s.setTarget(-21);

  EXPECT_EQ ( -21, s.getNext() );
  EXPECT_EQ ( -21, s.getNext() );
}
