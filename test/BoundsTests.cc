#include <gtest/gtest.h>
#include "helpers.hh"

#include "../geometry/Bounds.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

void overlapsTest(bool expectSame, Bounds2i const& b1, Bounds2i const& b2)
{
  Bounds2i b1flipped(b1.min().y(), b1.min().x(), b1.max().y(), b1.max().x());
  Bounds2i b2flipped(b2.min().y(), b2.min().x(), b2.max().y(), b2.max().x());

  if (expectSame)
  {
    EXPECT_TRUE ( b1.overlaps(b2) );
    EXPECT_TRUE ( b2.overlaps(b1) );

    EXPECT_TRUE ( b1flipped.overlaps(b2flipped) );
    EXPECT_TRUE ( b2flipped.overlaps(b1flipped) );
  }
  else
  {
    EXPECT_FALSE ( b1.overlaps(b2) );
    EXPECT_FALSE ( b2.overlaps(b1) );

    EXPECT_FALSE ( b1flipped.overlaps(b2flipped) );
    EXPECT_FALSE ( b2flipped.overlaps(b1flipped) );
  }
}

TEST (BoundsTests, overlaps)
{
  overlapsTest(true, Bounds2i(0,0, 10,10), Bounds2i(0,0, 10,10)); // identical
  overlapsTest(true, Bounds2i(0,0, 10,10), Bounds2i(5,5, 15,15));
  overlapsTest(true, Bounds2i(0,0, 10,10), Bounds2i(0,0, 5,5));
  overlapsTest(true, Bounds2i(0,0, 10,10), Bounds2i(5,5, 10,10));
  overlapsTest(true, Bounds2i(0,0, 10,10), Bounds2i(1,1, 9,9)); // contained
  overlapsTest(true, Bounds2i(16,280, 119,450), Bounds2i(13,444,17,449) );

  overlapsTest(false, Bounds2i(0,0, 10,10), Bounds2i(11,11, 15,15) );
  overlapsTest(false, Bounds2i(0,0, 10,10), Bounds2i(-2,-2, -1,-1) );
  overlapsTest(false, Bounds2i(0,0, 10,10), Bounds2i(1,11, 12, 20) );
}

TEST (BoundsTests, mid)
{
  EXPECT_TRUE ( VectorsEqual (Vector2i(5, 5), Bounds2i(0,0, 10,10).mid()) );
  EXPECT_TRUE ( VectorsEqual (Vector2d(5, 5), Bounds2d(Vector2d(0,0), Vector2d(10,10)).mid()) );
}
