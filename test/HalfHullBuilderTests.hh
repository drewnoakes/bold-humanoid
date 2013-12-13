#include "gtest/gtest.h"

#include "../geometry/halfhullbuilder.hh"

#include <vector>

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (HalfHullBuilderTests, topUnchanged)
{
  HalfHullBuilder<int> builder;
  vector<Vector2i> input = { {0,0}, {1,1}, {2,1.5} };

  vector<Vector2i> hull = builder.findHalfHull(input, HalfHull::Top);

  EXPECT_EQ( 3, hull.size() );
  EXPECT_EQ( input[0], hull[0] );
  EXPECT_EQ( input[1], hull[1] );
  EXPECT_EQ( input[2], hull[2] );
}

TEST (HalfHullBuilderTests, topChanged)
{
  HalfHullBuilder<int> builder;
  vector<Vector2i> input = { {0,1}, {1,0}, {2,1.5} };

  vector<Vector2i> hull = builder.findHalfHull(input, HalfHull::Top);

  EXPECT_EQ( 2, hull.size() );
  EXPECT_EQ( input[0], hull[0] );
  // input[1] is skipped
  EXPECT_EQ( input[2], hull[1] );
}

TEST (HalfHullBuilderTests, bottomUnchanged)
{
  HalfHullBuilder<int> builder;
  vector<Vector2i> input = { {0,1}, {1,0}, {2,1.5} };

  vector<Vector2i> hull = builder.findHalfHull(input, HalfHull::Bottom);

  EXPECT_EQ( 3, hull.size() );
  EXPECT_EQ( input[0], hull[0] );
  EXPECT_EQ( input[1], hull[1] );
  EXPECT_EQ( input[2], hull[2] );
}

TEST (HalfHullBuilderTests, bottomChanged)
{
  HalfHullBuilder<int> builder;
  vector<Vector2i> input = { {0,0}, {1,1}, {2,1.5} };

  vector<Vector2i> hull = builder.findHalfHull(input, HalfHull::Bottom);

  EXPECT_EQ( 2, hull.size() );
  EXPECT_EQ( input[0], hull[0] );
  // input[1] is skipped
  EXPECT_EQ( input[2], hull[1] );
}

TEST (HalfHullBuilderTests, emptyInput)
{
  HalfHullBuilder<int> builder;
  vector<Vector2i> input;

  EXPECT_EQ( 0, builder.findHalfHull(input, HalfHull::Top).size() );
  EXPECT_EQ( 0, builder.findHalfHull(input, HalfHull::Bottom).size() );
}
