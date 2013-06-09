#include "gtest/gtest.h"

#include "helpers.hh"

#include "../MotionTask/motiontask.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (JointSelectionTests, constructor)
{
  JointSelection sel = JointSelection(true, false, false);
  
  EXPECT_TRUE( sel.hasHead() );
  EXPECT_FALSE( sel.hasArms() );
  EXPECT_FALSE( sel.hasLegs() );
  
  sel = JointSelection(false, true, false);
  
  EXPECT_FALSE( sel.hasHead() );
  EXPECT_TRUE( sel.hasArms() );
  EXPECT_FALSE( sel.hasLegs() );
  
  sel = JointSelection(false, false, true);
  
  EXPECT_FALSE( sel.hasHead() );
  EXPECT_FALSE( sel.hasArms() );
  EXPECT_TRUE( sel.hasLegs() );
}

TEST (JointSelectionTests, indexer)
{
  JointSelection sel = JointSelection(true, false, false);
  
  EXPECT_TRUE( sel[(uchar)JointId::HEAD_PAN] );
  EXPECT_FALSE( sel[(uchar)JointId::L_ELBOW] );
  EXPECT_FALSE( sel[(uchar)JointId::L_HIP_PITCH] );
  
  sel = JointSelection(false, true, false);
  
  EXPECT_FALSE( sel[(uchar)JointId::HEAD_PAN] );
  EXPECT_TRUE( sel[(uchar)JointId::L_ELBOW] );
  EXPECT_FALSE( sel[(uchar)JointId::L_HIP_PITCH] );
  
  sel = JointSelection(false, false, true);
  
  EXPECT_FALSE( sel[(uchar)JointId::HEAD_PAN] );
  EXPECT_FALSE( sel[(uchar)JointId::L_ELBOW] );
  EXPECT_TRUE( sel[(uchar)JointId::L_HIP_PITCH] );
}
