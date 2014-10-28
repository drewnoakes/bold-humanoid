#include <gtest/gtest.h>
#include "helpers.hh"
#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (BlobTests, runSetToBlob)
{
  set<bold::Run> runSet;
  runSet.insert(bold::Run(4,5,0));
  runSet.insert(bold::Run(3,6,1));
  runSet.insert(bold::Run(2,7,2));
  runSet.insert(bold::Run(1,8,3));
  runSet.insert(bold::Run(0,9,4));
  runSet.insert(bold::Run(1,8,5));
  runSet.insert(bold::Run(2,7,6));
  runSet.insert(bold::Run(3,6,7));
  runSet.insert(bold::Run(4,5,8));

  auto blob = BlobDetectPass::runSetToBlob(runSet);

  EXPECT_EQ ( 50, blob.area );
  EXPECT_EQ ( Vector2d(4.5, 4), blob.mean );
  EXPECT_EQ ( Vector2i(0,0), blob.ul );
  EXPECT_EQ ( Vector2i(9,8), blob.br );
  EXPECT_EQ ( Bounds2i(0,0, 9,8), blob.bounds() );
}

TEST (BlobTests, compare)
{
  Blob blob1(Vector2i(0,0), Vector2i(10,10),
             121,
             Vector2d(5,5),// Matrix2f::Identity(),
             set<bold::Run>());
  Blob blob2(Vector2i(0,0), Vector2i(6,6),
             49,
             Vector2d(3,3), //Matrix2f::Identity(),
             set<bold::Run>());
  Blob blob3(Vector2i(3,3), Vector2i(9,9),
             49,
             Vector2d(6,6), //Matrix2f::Identity(),
             set<bold::Run>());

  EXPECT_TRUE ( blob2 < blob1 ); // Smaller area
  EXPECT_TRUE ( blob3 < blob1 ); // Smaller area
  EXPECT_TRUE ( blob2 < blob3 ); // Below
}

TEST (BlobTests, merge)
{
  Blob b1(Vector2i(10,10), Vector2i(20,20), 10, Vector2d(15,15), set<bold::Run>());
  Blob b2(Vector2i( 0, 0), Vector2i(10,10), 5 , Vector2d( 5, 5), set<bold::Run>());

  b1.merge(b2);

  EXPECT_EQ(15, b1.area);
  EXPECT_TRUE(VectorsEqual(Vector2d(11.666666, 11.666666), b1.mean));
  EXPECT_EQ(Vector2i( 0, 0), b1.ul);
  EXPECT_EQ(Vector2i(20,20), b1.br);
}
