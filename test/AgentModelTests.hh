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

  shared_ptr<Limb const> leftFoot = am.getLimb("lFoot");
  shared_ptr<Limb const> rightFoot = am.getLimb("rFoot");

  EXPECT_EQ( 0, am.mx28States[0].presentPosition );

  // Forces recalculation of transforms
  am.updatePosture();

  EXPECT_EQ( Vector3d(0.074/2, -0.1222 - 0.093 - 0.093 - 0.0335, -0.005),
	     Vector3d(leftFoot->transform.translation()) );
  EXPECT_TRUE( (leftFoot->transform.rotation() - Matrix3d::Identity()).isZero() );

  EXPECT_EQ( Vector3d(-0.074/2, -0.1222 - 0.093 - 0.093 - 0.0335, -0.005),
	     Vector3d(rightFoot->transform.translation()) );
  EXPECT_TRUE( (rightFoot->transform.rotation() - Matrix3d::Identity()).isZero() );

  //
  // Roll the legs out 90 degrees and check again
  //

  am.mx28States[JointData::ID_L_HIP_ROLL].presentPosition = M_PI/2;
  am.mx28States[JointData::ID_R_HIP_ROLL].presentPosition = M_PI/2;

  am.updatePosture();

  EXPECT_EQ( Vector3d(0.074/2 + 0.093 + 0.093 + 0.0335, -0.1222, -0.005),
	     Vector3d(leftFoot->transform.translation()) );
  EXPECT_EQ( AngleAxisd(M_PI/2, Vector3d::UnitZ()).matrix(), 
	     leftFoot->transform.rotation().matrix() );

  EXPECT_EQ( Vector3d(-(0.074/2 + 0.093 + 0.093 + 0.0335), -0.1222, -0.005),
	     Vector3d(rightFoot->transform.translation()) );
  EXPECT_EQ( AngleAxisd(-M_PI/2, Vector3d::UnitZ()).matrix(), 
	     rightFoot->transform.rotation().matrix() );
}
