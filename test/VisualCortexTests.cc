#include <gtest/gtest.h>

#include "../geometry/Bounds.hh"
#include "../VisualCortex/visualcortex.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (VisualCortexTests, shouldMergeBallBlobs)
{
  EXPECT_TRUE(VisualCortex::shouldMergeBallBlobs(
    Bounds2<ushort>(130,77,188,126),
    Bounds2<ushort>(120,93,174,137)
  ));
}
