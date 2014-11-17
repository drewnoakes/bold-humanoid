#include <gtest/gtest.h>

#include "../geometry/Bounds.hh"
#include "../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (Bounds2iTests, contains)
{
  EXPECT_TRUE  ( Bounds2i(0, 0, 10, 10).contains( Vector2i(5, 5) ) );
  EXPECT_TRUE  ( Bounds2i(0, 0, 10, 10).contains( Vector2i(0, 0) ) );
  EXPECT_TRUE  ( Bounds2i(0, 0, 10, 10).contains( Vector2i(10, 10) ) );

  EXPECT_FALSE ( Bounds2i(0, 0, 10, 10).contains( Vector2i(15, 15) ) );
  EXPECT_FALSE ( Bounds2i(0, 0, 10, 10).contains( Vector2i(-1, -1) ) );
  EXPECT_FALSE ( Bounds2i(0, 0, 10, 10).contains( Vector2i(-1, 5) ) );
  EXPECT_FALSE ( Bounds2i(0, 0, 10, 10).contains( Vector2i(5, 15) ) );
}

TEST (Bounds2iTests, widthAndHeight)
{
  EXPECT_EQ ( 0, Bounds2i ( 0, 0, 0,  0 ).width() );
  EXPECT_EQ ( 0, Bounds2i ( 0, 0, 0, 10 ).width() );
  EXPECT_EQ ( 3, Bounds2i ( 1, 1, 4,  2 ).width() );

  EXPECT_EQ ( 0, Bounds2i ( 0, 0,  0, 0 ).height() );
  EXPECT_EQ ( 0, Bounds2i ( 0, 0, 10, 0 ).height() );
  EXPECT_EQ ( 3, Bounds2i ( 1, 1, 2,  4 ).height() );
}

TEST (Bounds2iTests, corners)
{
  auto corners = Bounds2i(0, 0, 1, 2).getCorners();

  EXPECT_EQ ( 4, corners.size() );

  EXPECT_EQ ( Vector2i(0, 0), corners[0] );
  EXPECT_EQ ( Vector2i(0, 2), corners[1] );
  EXPECT_EQ ( Vector2i(1, 2), corners[2] );
  EXPECT_EQ ( Vector2i(1, 0), corners[3] );
}

TEST (Bounds2iTests, edges)
{
  auto edges = Bounds2i(0, 0, 1, 2).getEdges();

  EXPECT_EQ ( 4, edges.size() );

  EXPECT_EQ ( LineSegment2i(1, 0, 0, 0), edges[0] );
  EXPECT_EQ ( LineSegment2i(0, 0, 0, 2), edges[1] );
  EXPECT_EQ ( LineSegment2i(0, 2, 1, 2), edges[2] );
  EXPECT_EQ ( LineSegment2i(1, 2, 1, 0), edges[3] );
}

TEST (Bounds2iTests, merge)
{
  EXPECT_EQ(
    Bounds2i(1, 2, 3, 4),
    Bounds2i::merge(Bounds2i(1, 2, 2, 3), Bounds2i(2, 3, 3, 4))
  );

  EXPECT_EQ(
    Bounds2i(0, 0, 3, 3),
    Bounds2i::merge(Bounds2i(0, 0, 3, 3), Bounds2i(1, 1, 2, 2))
  );
}

TEST (Bounds2iTests, minDimension)
{
  EXPECT_EQ(10, Bounds2i(0,0,10,10).minDimension());
  EXPECT_EQ(3, Bounds2i(5,7,10,10).minDimension());
  EXPECT_EQ(3, Bounds2i(7,5,10,10).minDimension());
  EXPECT_EQ(0, Bounds2i(10,10,10,10).minDimension());
}

TEST (Bounds2iTests, maxDimension)
{
  EXPECT_EQ(10, Bounds2i(0,0,10,10).maxDimension());
  EXPECT_EQ(5, Bounds2i(5,7,10,10).maxDimension());
  EXPECT_EQ(5, Bounds2i(7,5,10,10).maxDimension());
  EXPECT_EQ(0, Bounds2i(10,10,10,10).maxDimension());
}
