#include <gtest/gtest.h>

#include "helpers.hh"
#include "../IncrementalRegression/incrementalregression.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (IncrementalRegressionTests, basic)
{
  IncrementalRegression regression;
  regression.setSqError(1.0);

  EXPECT_EQ( regression.getNPoints(), 0 );

  auto point1 = Vector2f{0, 0};
  auto point2 = Vector2f{1, 1};
  auto point3 = Vector2f{2, 3};

  regression.addPoint(point1);
  EXPECT_EQ( regression.getNPoints(), 1);
  EXPECT_EQ( regression.head(), point1 );

  regression.addPoint(point2);
  regression.solve();
  EXPECT_EQ( regression.getNPoints(), 2);
  EXPECT_EQ( regression.head(), point2 );

  EXPECT_TRUE( regression.fit(point3) < 1.0f );
  
  auto lineSegment = regression.getLineSegment();

  EXPECT_EQ( lineSegment.p1(), point1 );
  EXPECT_EQ( lineSegment.p2(), point2 );

  EXPECT_EQ( regression.fit(Vector2f{0, 0}), 0 );
  EXPECT_EQ( regression.fit(Vector2f{0.5, 0.5}), 0 );
  EXPECT_EQ( regression.fit(Vector2f{1, 1}), 0 );
  EXPECT_EQ( regression.fit(Vector2f{2, 2}), 0 );
}
