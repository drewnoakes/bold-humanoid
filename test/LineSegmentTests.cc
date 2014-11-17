#include <gtest/gtest.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "helpers.hh"
#include "../geometry/Bounds.hh"
#include "../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (LineSegmentTests, delta)
{
  EXPECT_EQ (Vector2i(50,100),  LineSegment2i(Vector2i(50,0),  Vector2i(100,100)).delta());
  EXPECT_EQ (Vector2i(-10,-20), LineSegment2i(Vector2i(40,30), Vector2i(30,10)).delta());
  EXPECT_EQ (Vector2i(2,1),     LineSegment2i(Vector2i(1,0),   Vector2i(3,1)).delta());
}

// This test commented as we now have a runtime assertion that fires here, rather than an exception
//TEST (LineSegmentTests, points_not_identical)
//{
//  ASSERT_THROW (LineSegment2i(Vector2i(50,0), Vector2i(50,0)), std::runtime_error);
//}

TEST (LineSegmentTests, gradient)
{
  EXPECT_EQ (0,   LineSegment2i(Vector2i(0,0), Vector2i(100,0)).gradient());
  EXPECT_EQ (1,   LineSegment2i(Vector2i(0,0), Vector2i(10,10)).gradient());
  EXPECT_EQ (-1,  LineSegment2i(Vector2i(0,0), Vector2i(10,-10)).gradient());
  EXPECT_EQ (0.5, LineSegment2i(Vector2i(1,0), Vector2i(3,1)).gradient());

  // Vertical lines have infinite gradient
  EXPECT_EQ (numeric_limits<double>::infinity(), LineSegment2i(Vector2i(0,0), Vector2i(0,10)).gradient());
}

TEST (LineSegmentTests, yIntersection)
{
  EXPECT_EQ (0,    LineSegment2i(Vector2i(0,0),   Vector2i(100,0)).yIntersection());
  EXPECT_EQ (10,   LineSegment2i(Vector2i(10,10), Vector2i(20,10)).yIntersection());
  EXPECT_EQ (-1,   LineSegment2i(Vector2i(1,0),   Vector2i(2,1)).yIntersection());
  EXPECT_EQ (-0.5, LineSegment2i(Vector2i(1,0),   Vector2i(3,1)).yIntersection());

  // Vertical lines have no yIntersection
  EXPECT_TRUE (std::isnan(LineSegment2i(Vector2i(10,0), Vector2i(10,10)).yIntersection()));
}

TEST (LineSegmentTests, angle)
{
  EXPECT_EQ (0,         LineSegment2i(Vector2i(0,0), Vector2i(1,0)).angle());
  EXPECT_EQ (M_PI/4,    LineSegment2i(Vector2i(0,0), Vector2i(1,1)).angle());
  EXPECT_EQ (M_PI/2,    LineSegment2i(Vector2i(1,0), Vector2i(1,1)).angle());
  EXPECT_EQ (M_PI,      LineSegment2i(Vector2i(1,1), Vector2i(0,1)).angle());
  EXPECT_EQ (atan(0.5), LineSegment2i(Vector2i(1,0), Vector2i(3,1)).angle());
}

TEST (LineSegmentTests, to)
{
  EXPECT_EQ ( LineSegment3i(Vector3i(1,2,0), Vector3i(3,4,0)),
              LineSegment2i(Vector2i(1,2),   Vector2i(3,4)  ).to<3>() );

  EXPECT_EQ ( LineSegment2i(Vector2i(1,2),   Vector2i(4,5)),
              LineSegment3i(Vector3i(1,2,3), Vector3i(4,5,6)).to<2>() );

  EXPECT_EQ ( LineSegment3i(Vector3i(1,2,3), Vector3i(4,5,6)),
              LineSegment3i(Vector3i(1,2,3), Vector3i(4,5,6)).to<3>() );
}

TEST (LineSegmentTests, cropTo)
{
  // Completely inside
  EXPECT_EQ( Maybe<LineSegment2i>(LineSegment2i(100, 100, 200, 200)), LineSegment2i(100, 100, 200, 200).cropTo(Bounds2i(0, 0, 300, 300)) );

  // Completely outside
  EXPECT_EQ( Maybe<LineSegment2i>(LineSegment2i(100, 100, 200, 200)), LineSegment2i(100, 100, 200, 200).cropTo(Bounds2i(0, 0, 300, 300)) );

  // Spanning right edge
  EXPECT_EQ( Maybe<LineSegment2i>(LineSegment2i(100, 100, 200, 100)), LineSegment2i(100, 100, 300, 100).cropTo(Bounds2i(0, 0, 200, 200)) );

  // Spanning left and right edge
  EXPECT_EQ( Maybe<LineSegment2i>(LineSegment2i(100, 100, 200, 100)), LineSegment2i(0, 100, 300, 100).cropTo(Bounds2i(100, 100, 200, 200)) );

  // Running through corners
  EXPECT_EQ( Maybe<LineSegment2i>(LineSegment2i(10, 10, 20, 20)), LineSegment2i(0, 0, 30, 30).cropTo(Bounds2i(10, 10, 20, 20)) );

  // Diagonally out via corner
  EXPECT_EQ( Maybe<LineSegment2i>(LineSegment2i(100, 100, 200, 200)), LineSegment2i(100, 100, 300, 300).cropTo(Bounds2i(0, 0, 200, 200)) );

  // Running along bottom edge
  EXPECT_EQ( Maybe<LineSegment2i>(LineSegment2i(50, 0, 100, 0)), LineSegment2i(0, 0, 200, 0).cropTo(Bounds2i(50, 0, 100, 100)) );
}

TEST (LineSegmentTests, tryIntersect2i)
{
  // Perpendicular
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(1, 1)), LineSegment2i(1, 0, 1, 2).tryIntersect(LineSegment2i(0, 1, 2, 1)) );

  // Perpendicular
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(5, 5)), LineSegment2i(0, 10, 10, 0).tryIntersect(LineSegment2i(0, 0, 10, 10)) );

  // Touching (V shape)
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(0, 0)), LineSegment2i(-1, 1, 0, 0).tryIntersect(LineSegment2i(0, 0, 1, 1)) );

  // Touching (T shape)
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(0, 1)), LineSegment2i(-1, 1, 1, 1).tryIntersect(LineSegment2i(0, 0, 0, 1)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(1, 1)), LineSegment2i(0, 1, 2, 1).tryIntersect(LineSegment2i(1, 1, 1, 2)) );

  // Touching (L shape)
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(-1, 1)), LineSegment2i(-2, 1, -1, 1).tryIntersect(LineSegment2i(-1, 1, -1, 2)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(-1, 1)), LineSegment2i(-1, 1, -2, 1).tryIntersect(LineSegment2i(-1, 2, -1, 1)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(-1, 1)), LineSegment2i(-2, 1, -1, 1).tryIntersect(LineSegment2i(-1, 2, -1, 1)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(1, 1)), LineSegment2i(1, 1, 1, 2).tryIntersect(LineSegment2i(1, 1, 2, 1)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(0, 0)), LineSegment2i(0, 0, 0, 1).tryIntersect(LineSegment2i(0, 0, 1, 0)) );

  // Parallel (vertical)
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(1, 1, 1, 2).tryIntersect(LineSegment2i(2, 0, 2, 1)) );
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(1, 1, 1, 2).tryIntersect(LineSegment2i(2, 0, 2, 1)) );

  // Parallel (horizontal)
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(0, 0, 1, 0).tryIntersect(LineSegment2i(0, 1, 1, 1)) );

  // Colinear
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(0, 0, 1, 0).tryIntersect(LineSegment2i(2, 0, 3, 0)) );
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(1, 1, 2, 2).tryIntersect(LineSegment2i(3, 3, 4, 4)) );
  // this case doesn't work -- is it important?
//EXPECT_EQ( Maybe<Vector2i>(Vector2i(2,2)), LineSegment2i(1, 1, 2, 2).tryIntersect(LineSegment2i(2, 2, 3, 3)) );

  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(0, 0, 0, 10).tryIntersect(LineSegment2i(5, 5, 15, 5)) );
}

TEST (LineSegmentTests, tryIntersect2d)
{
  // Perpendicular
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(1, 1)), LineSegment2d(1, 0, 1, 2).tryIntersect(LineSegment2d(0, 1, 2, 1)) );

  // Perpendicular
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(5, 5)), LineSegment2d(0, 10, 10, 0).tryIntersect(LineSegment2d(0, 0, 10, 10)) );

  // Touching (V shape)
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(0, 0)), LineSegment2d(-1, 1, 0, 0).tryIntersect(LineSegment2d(0, 0, 1, 1)) );

  // Touching (T shape)
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(0, 1)), LineSegment2d(-1, 1, 1, 1).tryIntersect(LineSegment2d(0, 0, 0, 1)) );
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(1, 1)), LineSegment2d(0, 1, 2, 1).tryIntersect(LineSegment2d(1, 1, 1, 2)) );

  // Touching (L shape)
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(-1, 1)), LineSegment2d(-2, 1, -1, 1).tryIntersect(LineSegment2d(-1, 1, -1, 2)) );
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(-1, 1)), LineSegment2d(-1, 1, -2, 1).tryIntersect(LineSegment2d(-1, 2, -1, 1)) );
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(-1, 1)), LineSegment2d(-2, 1, -1, 1).tryIntersect(LineSegment2d(-1, 2, -1, 1)) );
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(1, 1)), LineSegment2d(1, 1, 1, 2).tryIntersect(LineSegment2d(1, 1, 2, 1)) );
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(0, 0)), LineSegment2d(0, 0, 0, 1).tryIntersect(LineSegment2d(0, 0, 1, 0)) );

  // Parallel (vertical)
  EXPECT_EQ( Maybe<Vector2d>::empty(), LineSegment2d(1, 1, 1, 2).tryIntersect(LineSegment2d(2, 0, 2, 1)) );
  EXPECT_EQ( Maybe<Vector2d>::empty(), LineSegment2d(1, 1, 1, 2).tryIntersect(LineSegment2d(2, 0, 2, 1)) );

  // Parallel (horizontal)
  EXPECT_EQ( Maybe<Vector2d>::empty(), LineSegment2d(0, 0, 1, 0).tryIntersect(LineSegment2d(0, 1, 1, 1)) );

  // Collinear
  EXPECT_EQ( Maybe<Vector2d>::empty(), LineSegment2d(0, 0, 1, 0).tryIntersect(LineSegment2d(2, 0, 3, 0)) );
  EXPECT_EQ( Maybe<Vector2d>::empty(), LineSegment2d(1, 1, 2, 2).tryIntersect(LineSegment2d(3, 3, 4, 4)) );
  // this case doesn't work -- is it important?
//EXPECT_EQ( Maybe<Vector2d>(Vector2d(2,2)), LineSegment2d(1, 1, 2, 2).tryIntersect(LineSegment2d(2, 2, 3, 3)) );

  EXPECT_EQ( Maybe<Vector2d>::empty(), LineSegment2d(0, 0, 0, 1).tryIntersect(LineSegment2d(0.5, 0.5, 1.5, 0.5)) );

  // 't' is the distance along the line upon which tryIntersect is called.
  // 'u' is the distance along the provided line.
  double t, u;
  EXPECT_EQ( Maybe<Vector2d>(Vector2d(0, 2)), LineSegment2d(0, 0, 0, 3).tryIntersect(LineSegment2d(-1, 2, 3, 2), t, u) );
  EXPECT_NEAR ( 2.0/3.0, t, 0.000001 );
  EXPECT_NEAR ( 1.0/4.0, u, 0.000001 );
}

TEST (LineSegmentTests, normalisedDot)
{
  EXPECT_EQ ( 0, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).normalisedDot(LineSegment2d(Vector2d(0,0),Vector2d(1,0))) );
  EXPECT_EQ ( 0, LineSegment2d(Vector2d(0,0),Vector2d(0,2)).normalisedDot(LineSegment2d(Vector2d(2,2),Vector2d(4,2))) );
  EXPECT_EQ ( -1, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).normalisedDot(LineSegment2d(Vector2d(0,0),Vector2d(0,-1))) );

  EXPECT_EQ ( 1, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).normalisedDot(LineSegment2d(Vector2d(0,0),Vector2d(0,1))) );

  EXPECT_NEAR ( 0.707107, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).normalisedDot(LineSegment2d(Vector2d(0,0),Vector2d(1,1))), 0.000001 );
}

TEST (LineSegmentTests, smallestAngleBetween)
{
  EXPECT_NEAR ( M_PI/2, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).smallestAngleBetween(LineSegment2d(Vector2d(0,0),Vector2d(1,0))), 0.000001 );
  EXPECT_NEAR ( M_PI/2, LineSegment2d(Vector2d(0,0),Vector2d(0,2)).smallestAngleBetween(LineSegment2d(Vector2d(2,2),Vector2d(4,2))), 0.000001 );

  EXPECT_NEAR ( 0, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).smallestAngleBetween(LineSegment2d(Vector2d(0,0),Vector2d(0,-1))), 0.000001 );

  EXPECT_NEAR ( M_PI/2, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).smallestAngleBetween(LineSegment2d(Vector2d(0,0),Vector2d(1,0))), 0.000001 );

  EXPECT_NEAR ( M_PI/4, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).smallestAngleBetween(LineSegment2d(Vector2d(0,0),Vector2d(1,1))), 0.000001 );
  EXPECT_NEAR ( M_PI/4, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).smallestAngleBetween(LineSegment2d(Vector2d(0,0),Vector2d(-1,1))), 0.000001 );
  EXPECT_NEAR ( M_PI/4, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).smallestAngleBetween(LineSegment2d(Vector2d(0,0),Vector2d(-1,-1))), 0.000001 );
  EXPECT_NEAR ( M_PI/4, LineSegment2d(Vector2d(0,0),Vector2d(0,1)).smallestAngleBetween(LineSegment2d(Vector2d(0,0),Vector2d(1,-1))), 0.000001 );
}

TEST (LineSegmentTests, addAndSubtractAVector)
{
  EXPECT_EQ (LineSegment2i(Vector2i(0,0), Vector2i(0,1)), LineSegment2i(Vector2i(0,0), Vector2i(0,1)) + Vector2i(0, 0));
  EXPECT_EQ (LineSegment2i(Vector2i(0,0), Vector2i(0,1)), LineSegment2i(Vector2i(0,0), Vector2i(0,1)) - Vector2i(0, 0));

  EXPECT_EQ (LineSegment2i(Vector2i(1,1),   Vector2i(1,2)),  LineSegment2i(Vector2i(0,0), Vector2i(0,1)) + Vector2i(1, 1));
  EXPECT_EQ (LineSegment2i(Vector2i(-1,-1), Vector2i(-1,0)), LineSegment2i(Vector2i(0,0), Vector2i(0,1)) - Vector2i(1, 1));

  EXPECT_EQ (LineSegment2i(Vector2i(1,1),   Vector2i(1,2)),  LineSegment2i(Vector2i(0,0), Vector2i(0,1)) - Vector2i(-1, -1));
  EXPECT_EQ (LineSegment2i(Vector2i(-1,-1), Vector2i(-1,0)), LineSegment2i(Vector2i(0,0), Vector2i(0,1)) + Vector2i(-1, -1));
}

TEST (LineSegmentTests, mid)
{
  EXPECT_TRUE(VectorsEqual(
    Vector2d(0.5,1),
    LineSegment2d(Vector2d(0,0), Vector2d(1,2)).mid()
  ));
}

/*
TEST (LineSegmentTests, toLine)
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
*/
