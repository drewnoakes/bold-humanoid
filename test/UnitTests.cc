#include "gtest/gtest.h"
#include <Eigen/Core>
#include <cmath>

#include "../Geometry/geometry.hh"

using namespace bold;
using namespace Eigen;

// TODO move to CMake and split this up into one file per class-under-test

//////////////////////////////////////////////////////////////////////////////

TEST (LineSegmentTests, delta)
{
  EXPECT_EQ (Vector2i(50,100),  LineSegment2i(Vector2i(50,0),  Vector2i(100,100)).delta());
  EXPECT_EQ (Vector2i(-10,-20), LineSegment2i(Vector2i(40,30), Vector2i(30,10)).delta());
  EXPECT_EQ (Vector2i(2,1),     LineSegment2i(Vector2i(1,0),   Vector2i(3,1)).delta());
}

TEST (LineSegmentTests, points_not_identical)
{
  ASSERT_THROW (LineSegment2i(Vector2i(50,0), Vector2i(50,0)), std::string);
}

TEST (LineSegmentTests, gradient)
{
  EXPECT_EQ (0,   LineSegment2i(Vector2i(0,0), Vector2i(100,0)).gradient());
  EXPECT_EQ (1,   LineSegment2i(Vector2i(0,0), Vector2i(10,10)).gradient());
  EXPECT_EQ (-1,  LineSegment2i(Vector2i(0,0), Vector2i(10,-10)).gradient());
  EXPECT_EQ (0.5, LineSegment2i(Vector2i(1,0), Vector2i(3,1)).gradient());

  // Vertical lines have infinite gradient
  EXPECT_EQ (FP_INFINITE, LineSegment2i(Vector2i(0,0), Vector2i(0,10)).gradient());
}

TEST (LineSegmentTests, yIntersection)
{
  EXPECT_EQ (0,    LineSegment2i(Vector2i(0,0),   Vector2i(100,0)).yIntersection());
  EXPECT_EQ (10,   LineSegment2i(Vector2i(10,10), Vector2i(20,10)).yIntersection());
  EXPECT_EQ (-1,   LineSegment2i(Vector2i(1,0),   Vector2i(2,1)).yIntersection());
  EXPECT_EQ (-0.5, LineSegment2i(Vector2i(1,0),   Vector2i(3,1)).yIntersection());

  // Vertical lines have no yIntersection
  EXPECT_EQ (FP_NAN, LineSegment2i(Vector2i(10,0), Vector2i(10,10)).yIntersection());
}

TEST (LineSegmentTests, angle)
{
  EXPECT_EQ (0,         LineSegment2i(Vector2i(0,0), Vector2i(1,0)).angle());
  EXPECT_EQ (M_PI/4,    LineSegment2i(Vector2i(0,0), Vector2i(1,1)).angle());
  EXPECT_EQ (M_PI/2,    LineSegment2i(Vector2i(1,0), Vector2i(1,1)).angle());
  EXPECT_EQ (M_PI,      LineSegment2i(Vector2i(1,1), Vector2i(0,1)).angle());
  EXPECT_EQ (atan(0.5), LineSegment2i(Vector2i(1,0), Vector2i(3,1)).angle());
}

TEST (SanityTests, tangents)
{
  EXPECT_EQ ( atan(1/2.0), atan2(1,2) );
  EXPECT_EQ ( atan(1/3.0), atan2(1,3) );
  EXPECT_EQ ( atan(1/4.0), atan2(1,4) );
}

TEST (DISABLED_LineSegmentTests, toLine)
{
  // Vertical line at x=10
  EXPECT_EQ (Line(10, 0), LineSegment2i(Vector2i(10,0), Vector2i(10,10)).toLine());

  // Vertical line at x=-10
  EXPECT_EQ (Line(-10, 0), LineSegment2i(Vector2i(-10,0), Vector2i(-10,10)).toLine());

  // Horizontal line at y=10
  EXPECT_EQ (Line(10, M_PI/2), LineSegment2i(Vector2i(0,10), Vector2i(10,10)).toLine());

  // Vertical line at x=-10
  EXPECT_EQ (Line(-10, M_PI/2), LineSegment2i(Vector2i(0,-10), Vector2i(10,-10)).toLine());

  // y = x
  EXPECT_EQ (Line(0, M_PI*3/4), LineSegment2i(Vector2i(0,0), Vector2i(1,1)).toLine());

  // y = -x
  EXPECT_EQ (Line(0, M_PI/4), LineSegment2i(Vector2i(0,0), Vector2i(1,-1)).toLine());

  // line from (1,0) to (2,1)
  EXPECT_EQ (Line(-sqrt(2)/2, M_PI*3/4), LineSegment2i(Vector2i(1,0), Vector2i(2,1)).toLine());

  // line from (1,0) to (3,1)
  EXPECT_EQ (Line(-sin(atan(0.5)), M_PI/2+atan(0.5)), LineSegment2i(Vector2i(1,0), Vector2i(3,1)).toLine());

  // line from (3,1) to (1,0) -- same as prior test, but in reverse order (shouldn't matter)
  EXPECT_EQ (Line(-sin(atan(0.5)), M_PI/2+atan(0.5)), LineSegment2i(Vector2i(3,1), Vector2i(1,0)).toLine());

  // line from (5.5,2) to (3,3.5)
  EXPECT_EQ (Line(-sin(atan(0.5)), M_PI+atan(-0.6)), LineSegment2i(Vector2i(5.5,2), Vector2i(3,3.5)).toLine());
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
