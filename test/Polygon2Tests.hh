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

  EXPECT_EQ ( 3, poly.vertexCount() );
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

  ASSERT_TRUE( poly.contains(Vector2i(0,0)) );
  ASSERT_TRUE( poly.contains(Vector2i(0,1)) );
  ASSERT_TRUE( poly.contains(Vector2i(1,1)) );

  ASSERT_FALSE( poly.contains(Vector2i(2,1)) );
  ASSERT_FALSE( poly.contains(Vector2i(2,2)) );
  ASSERT_FALSE( poly.contains(Vector2i(3,2)) );
  ASSERT_FALSE( poly.contains(Vector2i(3,3)) );
  ASSERT_FALSE( poly.contains(Vector2i(2,3)) );
  ASSERT_FALSE( poly.contains(Vector2i(0,3)) );
  ASSERT_FALSE( poly.contains(Vector2i(3,0)) );
  ASSERT_FALSE( poly.contains(Vector2i(-1,-1)) );
  ASSERT_FALSE( poly.contains(Vector2i(-1,1)) );
  ASSERT_FALSE( poly.contains(Vector2i(1,-1)) );
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

  ASSERT_TRUE( poly.contains(Vector2f(0,0)) );
  ASSERT_TRUE( poly.contains(Vector2f(0,1)) );
  ASSERT_TRUE( poly.contains(Vector2f(1,1)) );
  ASSERT_TRUE( poly.contains(Vector2f(1.99999,1)) );
  ASSERT_TRUE( poly.contains(Vector2f(1.99999,1.99999)) );

  ASSERT_FALSE( poly.contains(Vector2f(2,1)) );
  ASSERT_FALSE( poly.contains(Vector2f(2,2)) );
  ASSERT_FALSE( poly.contains(Vector2f(3,2)) );
  ASSERT_FALSE( poly.contains(Vector2f(3,3)) );
  ASSERT_FALSE( poly.contains(Vector2f(2,3)) );
  ASSERT_FALSE( poly.contains(Vector2f(0,3)) );
  ASSERT_FALSE( poly.contains(Vector2f(3,0)) );
  ASSERT_FALSE( poly.contains(Vector2f(-1,-1)) );
  ASSERT_FALSE( poly.contains(Vector2f(-1,1)) );
  ASSERT_FALSE( poly.contains(Vector2f(1,-1)) );
}

TEST (Polygon2Tests, double_triangleContains)
{
  vector<Vector2d> points = {
    Vector2d(0,0),
    Vector2d(1,0),
    Vector2d(0,1)
  };

  Polygon2<double> poly(points);

  ASSERT_TRUE( poly.contains(Vector2d(0,0)) );
  ASSERT_TRUE( poly.contains(Vector2d(0.0001,0.0001)) );
  ASSERT_TRUE( poly.contains(Vector2d(0.0005,0.9990)) );
  ASSERT_TRUE( poly.contains(Vector2d(0.9990,0.0005)) );
  ASSERT_TRUE( poly.contains(Vector2d(0.4999,0.4999)) );

  ASSERT_FALSE( poly.contains(Vector2d(0.0005,1)) );
  ASSERT_FALSE( poly.contains(Vector2d(1,0.0005)) );
  ASSERT_FALSE( poly.contains(Vector2d(0.5001,0.5001)) );
  ASSERT_FALSE( poly.contains(Vector2d(1,1)) );
  ASSERT_FALSE( poly.contains(Vector2d(-0.0001,-0.0001)) );
  ASSERT_FALSE( poly.contains(Vector2d(-0.0001,0.0001)) );
  ASSERT_FALSE( poly.contains(Vector2d(0.0001,-0.0001)) );
}
