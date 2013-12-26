#include "gtest/gtest.h"
#include "helpers.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>

#include "../geometry/Polygon2.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (Polygon2Tests, basics)
{
  vector<Vector2i> points = {
    Vector2i(0,0),
    Vector2i(0,1),
    Vector2i(1,0)
  };

  Polygon2<int> poly(points);

  ASSERT_EQ ( 3, poly.vertexCount() );
  EXPECT_TRUE ( VectorsEqual(Vector2i(0,0), poly[0] ));
  EXPECT_TRUE ( VectorsEqual(Vector2i(0,1), poly[1] ));
  EXPECT_TRUE ( VectorsEqual(Vector2i(1,0), poly[2] ));
}

TEST (Polygon2Tests, int_squareContains)
{
  vector<Vector2i> points = {
    Vector2i(0,0),
    Vector2i(0,2),
    Vector2i(2,2),
    Vector2i(2,0)
  };

  Polygon2<int> poly(points);

  EXPECT_TRUE( poly.contains(Vector2i(0,0)) );
  EXPECT_TRUE( poly.contains(Vector2i(0,1)) );
  EXPECT_TRUE( poly.contains(Vector2i(1,1)) );

  EXPECT_FALSE( poly.contains(Vector2i(2,1)) );
  EXPECT_FALSE( poly.contains(Vector2i(2,2)) );
  EXPECT_FALSE( poly.contains(Vector2i(3,2)) );
  EXPECT_FALSE( poly.contains(Vector2i(3,3)) );
  EXPECT_FALSE( poly.contains(Vector2i(2,3)) );
  EXPECT_FALSE( poly.contains(Vector2i(0,3)) );
  EXPECT_FALSE( poly.contains(Vector2i(3,0)) );
  EXPECT_FALSE( poly.contains(Vector2i(-1,-1)) );
  EXPECT_FALSE( poly.contains(Vector2i(-1,1)) );
  EXPECT_FALSE( poly.contains(Vector2i(1,-1)) );
}

TEST (Polygon2Tests, float_squareContains)
{
  vector<Vector2f> points = {
    Vector2f(0,0),
    Vector2f(0,2),
    Vector2f(2,2),
    Vector2f(2,0)
  };

  Polygon2<float> poly(points);

  EXPECT_TRUE( poly.contains(Vector2f(0,0)) );
  EXPECT_TRUE( poly.contains(Vector2f(0,1)) );
  EXPECT_TRUE( poly.contains(Vector2f(1,1)) );
  EXPECT_TRUE( poly.contains(Vector2f(1.99999,1)) );
  EXPECT_TRUE( poly.contains(Vector2f(1.99999,1.99999)) );

  EXPECT_FALSE( poly.contains(Vector2f(2,1)) );
  EXPECT_FALSE( poly.contains(Vector2f(2,2)) );
  EXPECT_FALSE( poly.contains(Vector2f(3,2)) );
  EXPECT_FALSE( poly.contains(Vector2f(3,3)) );
  EXPECT_FALSE( poly.contains(Vector2f(2,3)) );
  EXPECT_FALSE( poly.contains(Vector2f(0,3)) );
  EXPECT_FALSE( poly.contains(Vector2f(3,0)) );
  EXPECT_FALSE( poly.contains(Vector2f(-1,-1)) );
  EXPECT_FALSE( poly.contains(Vector2f(-1,1)) );
  EXPECT_FALSE( poly.contains(Vector2f(1,-1)) );
}

TEST (Polygon2Tests, double_diamondContains)
{
  vector<Vector2d> points = {
    Vector2d(0,0),
    Vector2d(-1,1),
    Vector2d(0,2),
    Vector2d(1,1)
  };

  Polygon2<double> poly(points);

  // vertices
  EXPECT_TRUE( poly.contains(Vector2d(-1,1)) );
  EXPECT_FALSE( poly.contains(Vector2d(0,0)) );
  EXPECT_FALSE( poly.contains(Vector2d(1,1)) );
  EXPECT_FALSE( poly.contains(Vector2d(0,2)) );

  // clearly inside
  EXPECT_TRUE( poly.contains(Vector2d(0,1)) );
  EXPECT_TRUE( poly.contains(Vector2d(0,0.001)) );

  // clearly outside
  EXPECT_FALSE( poly.contains(Vector2d(1,0)) );
  EXPECT_FALSE( poly.contains(Vector2d(1,1)) );
}

TEST (Polygon2Tests, double_triangleContains)
{
  vector<Vector2d> points = {
    Vector2d(0,0),
    Vector2d(1,0),
    Vector2d(0,1)
  };

  Polygon2<double> poly(points);

  EXPECT_TRUE( poly.contains(Vector2d(0,0)) );
  EXPECT_TRUE( poly.contains(Vector2d(0.0001,0.0001)) );
  EXPECT_TRUE( poly.contains(Vector2d(0.0005,0.9990)) );
  EXPECT_TRUE( poly.contains(Vector2d(0.9990,0.0005)) );
  EXPECT_TRUE( poly.contains(Vector2d(0.4999,0.4999)) );

  EXPECT_FALSE( poly.contains(Vector2d(0.0005,1)) );
  EXPECT_FALSE( poly.contains(Vector2d(1,0.0005)) );
  EXPECT_FALSE( poly.contains(Vector2d(0.5001,0.5001)) );
  EXPECT_FALSE( poly.contains(Vector2d(1,1)) );
  EXPECT_FALSE( poly.contains(Vector2d(-0.0001,-0.0001)) );
  EXPECT_FALSE( poly.contains(Vector2d(-0.0001,0.0001)) );
  EXPECT_FALSE( poly.contains(Vector2d(0.0001,-0.0001)) );
}

TEST (Polygon2Tests, clip)
{
  // unit square
  vector<Vector2d> points = {
    Vector2d(0,0),
    Vector2d(1,0),
    Vector2d(1,1),
    Vector2d(0,1)
  };

  Polygon2<double> poly(points);

  // Exactly corner to corner, diagonally (touching all four line segments)
  auto clipped = poly.clipLine(LineSegment2d(Vector2d(0,0),Vector2d(1,1)));
  ASSERT_TRUE(clipped.hasValue());
  ASSERT_TRUE(LinesEqual(
    LineSegment2d(Vector2d(0,0),Vector2d(1,1)),
    clipped.value()
  ));

  // Inside, completely contained
  clipped = poly.clipLine(LineSegment2d(Vector2d(0.25,0.25),Vector2d(0.75,0.75)));
  ASSERT_TRUE(clipped.hasValue());
  ASSERT_TRUE(LinesEqual(
    LineSegment2d(Vector2d(0.25,0.25),Vector2d(0.75,0.75)),
    clipped.value()
  ));

  // From inside (corner) to outside, running through diagonally opposite corner (touching all four line segments)
  clipped = poly.clipLine(LineSegment2d(Vector2d(0,0),Vector2d(5,5)));
  ASSERT_TRUE(clipped.hasValue());
  ASSERT_TRUE(LinesEqual(
    LineSegment2d(Vector2d(0,0),Vector2d(1,1)),
    clipped.value()
  ));

  // Completely inside to outside, touching one line segment
  clipped = poly.clipLine(LineSegment2d(Vector2d(0.5,0.5),Vector2d(1.5,0.5)));
  ASSERT_TRUE(clipped.hasValue());
  ASSERT_TRUE(LinesEqual(
    LineSegment2d(Vector2d(0.5,0.5),Vector2d(1,0.5)),
    clipped.value()
  ));

  // Run horizontally through cube, intersecting both left and right edges
  clipped = poly.clipLine(LineSegment2d(Vector2d(-1,0.5),Vector2d(2,0.5)));
  ASSERT_TRUE(clipped.hasValue());
  ASSERT_TRUE(LinesEqual(
    LineSegment2d(Vector2d(0,0.5),Vector2d(1,0.5)),
    clipped.value()
  ));

  // Run diagonally through cube, bisecting the top left quadrant
  clipped = poly.clipLine(LineSegment2d(Vector2d(-0.5,0),Vector2d(1,1.5)));
  ASSERT_TRUE(clipped.hasValue());
  ASSERT_TRUE(LinesEqual(
    LineSegment2d(Vector2d(0,0.5),Vector2d(0.5,1)),
    clipped.value()
  ));
}
