#include "gtest/gtest.h"

#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (BlobTests, runSetToBlob)
{
  set<bold::Run> runSet;
  runSet.insert(bold::Run(0,10,0));
  runSet.insert(bold::Run(0,10,1));
  runSet.insert(bold::Run(0,10,1));
  runSet.insert(bold::Run(0,10,1));
}
