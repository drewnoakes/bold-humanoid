#include "gtest/gtest.h"

#include "../AgentModel/agentmodel.hh"

using namespace std;
using namespace bold;
using namespace Eigen;
using namespace Robot;

TEST (AgentModelTests, posture)
{
  auto& am = AgentModel::getInstance();

  am.initialise();

  EXPECT_EQ( 0, am.mx28States[0].presentPosition );

  // Forces recalculation of transforms
  am.updatePosture();

  shared_ptr<Limb const> leftFoot = am.getLeftFoot();

  EXPECT_EQ( Vector3d(0.074/2, -0.1222 - 0.093 - 0.093 - 0.0335, -0.005), leftFoot->transform.translation() );

  EXPECT_TRUE( (leftFoot->transform.rotation() - Matrix3d::Identity()).isZero() );

  //
  // Roll the leg out and check again
  //

  am.mx28States[JointData::ID_L_HIP_ROLL].presentPosition = M_PI/2;

  am.updatePosture();

  EXPECT_EQ( Vector3d(0.074/2 + 0.093 + 0.093 + 0.0335, -0.1222, -0.005), leftFoot->transform.translation() );

  EXPECT_EQ( AngleAxisd(M_PI/2, Vector3d::UnitZ()).matrix(), leftFoot->transform.rotation().matrix() );
}