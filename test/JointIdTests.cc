#include "gtest/gtest.h"

#include "../JointId/jointid.hh"

using namespace std;
using namespace bold;

TEST (JointPairsTests, getPartner)
{
  EXPECT_EQ(JointId::R_SHOULDER_PITCH, JointPairs::getPartner(JointId::L_SHOULDER_PITCH));
  EXPECT_EQ(JointId::R_HIP_PITCH, JointPairs::getPartner(JointId::L_HIP_PITCH));
  EXPECT_EQ(JointId::L_KNEE, JointPairs::getPartner(JointId::R_KNEE));
}

TEST (JointPairsTests, isBase)
{
  EXPECT_TRUE ( JointPairs::isBase(JointId::R_SHOULDER_PITCH) );
  EXPECT_FALSE( JointPairs::isBase(JointId::L_SHOULDER_PITCH) );
  EXPECT_TRUE ( JointPairs::isBase(JointId::R_HIP_PITCH) );
  EXPECT_FALSE( JointPairs::isBase(JointId::L_HIP_PITCH) );
  EXPECT_TRUE ( JointPairs::isBase(JointId::R_KNEE) );
}
