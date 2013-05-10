#include "gtest/gtest.h"

#include "../StateObject/BodyState/bodystate.hh"
#include "../JointId/jointid.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (BodyStateTests, posture)
{
  double angles[21] = {0,};

  //
  // Create a body with all hinges at an angle of zero
  //

  auto am1 = BodyState(angles);
  shared_ptr<Limb const> leftFoot = am1.getLimb("lFoot");
  shared_ptr<Limb const> rightFoot = am1.getLimb("rFoot");

  EXPECT_EQ( Vector3d(-0.074/2, -0.005, -0.1222 - 0.093 - 0.093 - 0.0335),
	     Vector3d(leftFoot->transform.translation()) );
  EXPECT_TRUE( (leftFoot->transform.rotation() - Matrix3d::Identity()).isZero() );

  EXPECT_EQ( Vector3d(0.074/2, -0.005, -0.1222 - 0.093 - 0.093 - 0.0335),
	     Vector3d(rightFoot->transform.translation()) );
  EXPECT_TRUE( (rightFoot->transform.rotation() - Matrix3d::Identity()).isZero() );

  //
  // Roll the legs out 90 degrees and check again
  //

  angles[(int)JointId::L_HIP_ROLL] = M_PI/2;
  angles[(int)JointId::R_HIP_ROLL] = M_PI/2;

  auto am2 = BodyState(angles);
  leftFoot = am2.getLimb("lFoot");
  rightFoot = am2.getLimb("rFoot");

  EXPECT_EQ( Vector3d(-0.074/2 - 0.093 - 0.093 - 0.0335, -0.005, -0.1222),
	     Vector3d(leftFoot->transform.translation()) );
  EXPECT_EQ( AngleAxisd(M_PI/2, Vector3d::UnitY()).matrix(),
	     leftFoot->transform.rotation().matrix() );

  EXPECT_EQ( Vector3d(0.074/2 + 0.093 + 0.093 + 0.0335, -0.005, -0.1222),
	     Vector3d(rightFoot->transform.translation()) );
  EXPECT_EQ( AngleAxisd(-M_PI/2, Vector3d::UnitY()).matrix(),
	     rightFoot->transform.rotation().matrix() );
}
