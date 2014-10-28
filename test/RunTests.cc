#include <gtest/gtest.h>

#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"

using namespace std;
using namespace bold;

bold::Run makeRun(unsigned startX, unsigned endX, unsigned y = 1)
{
  Run run(startX, y);
  run.endX = endX;

  EXPECT_EQ ( y, run.y );
  EXPECT_EQ ( startX, run.startX );
  EXPECT_EQ ( endX, run.endX );
  EXPECT_EQ ( endX - startX + 1, run.length() );

  return run;
}

TEST (RunTests, length)
{
  EXPECT_EQ ( 1, makeRun(0, 0).length() );
  EXPECT_EQ ( 1, makeRun(1, 1).length() );
  EXPECT_EQ ( 2, makeRun(1, 2).length() );
  EXPECT_EQ ( 3, makeRun(1, 3).length() );
}

TEST (RunTests, overlaps)
{
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(5, 15)) );
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(15, 25)) );
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(11, 19)) );
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(0, 10)) );
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(10, 10)) );
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(20, 20)) );
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(20, 30)) );
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(0, 30)) );
  EXPECT_TRUE ( makeRun(10, 10).overlaps(makeRun(10, 10)) );

  // touching at a diagonal (8 connected)
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(21, 30)) );
  EXPECT_TRUE ( makeRun(10, 20).overlaps(makeRun(0, 9)) );

  // completely separate
  EXPECT_FALSE ( makeRun(10, 20).overlaps(makeRun(22, 30)) );
  EXPECT_FALSE ( makeRun(10, 20).overlaps(makeRun(0, 8)) );
}
