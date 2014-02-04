#include "gtest/gtest.h"

#include "../StateObject/BodyState/bodystate.hh"
#include "../JointId/jointid.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (BodyStateTests, posture)
{
  double angles[21] = {0,};
  std::vector<int> positionValueDiffs(21, 0);

  //
  // Create a body with all hinges at an angle of zero
  //

  auto am1 = BodyState(angles, positionValueDiffs, 1);
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

  auto am2 = BodyState(angles, positionValueDiffs, 2);
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

TEST (BodyStateTests, camera)
{
  double angles[22] = {0,};
  std::vector<int> positionValueDiffs(21, 0);

  Matrix3d expectedCameraRotation = AngleAxisd(M_PI/4, Vector3d::UnitY()).matrix();

  auto body = BodyState(angles, positionValueDiffs, 3);

//   Matrix3d actualCameraTorsoRotation = body.getLimb("camera")->transform.rotation().matrix();

//   EXPECT_EQ(Vector3d(0, 0.0332, 0.0505 + 0.0344),
//             Vector3d(body.getLimb("camera")->transform.translation()));

//   EXPECT_EQ(expectedCameraRotation, actualCameraTorsoRotation);

  // From DARwIn-OP_Kinematics.pdf
  double cameraHeight = (33.5 + 93 + 93 + 122.2 + 50.5 + 34.4) / 1000.0;

  EXPECT_EQ(0.4266, cameraHeight);

//   EXPECT_EQ(Vector3d(0, 0.0332, cameraHeight),
//             Vector3d(body.getCameraAgentTransform().translation()));

  Matrix3d actualCameraAgentRotation = body.getCameraAgentTransform().rotation().matrix();

  cout << "Expected:" << endl;
  cout << expectedCameraRotation << endl;

  cout << "Actual:" << endl;
  cout << actualCameraAgentRotation << endl;

//   EXPECT_EQ(expectedCameraRotation, actualCameraAgentRotation);
}
