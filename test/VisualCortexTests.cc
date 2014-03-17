#include "gtest/gtest.h"


#include "../VisualCortex/visualcortex.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (VisualCortexTests, shouldMergeBallBlobs)
{
  EXPECT_TRUE(VisualCortex::shouldMergeBallBlobs(
    Bounds2i(130,77,188,126),
    Bounds2i(120,93,174,137)
  ));
}
