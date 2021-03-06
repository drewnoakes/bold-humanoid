#include <gtest/gtest.h>

#include "helpers.hh"

#include "../BodyModel/DarwinBodyModel/darwinbodymodel.hh"
#include "../JointId/jointid.hh"
#include "../Math/math.hh"
#include "../StateObject/BodyState/bodystate.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

// See DARwIn-OP_Kinematics.pdf

const double footSide = 0.074/2;
const double footForward = -0.005;
const double legLength = 0.093 + 0.093 + 0.0335;
const double footDownward = 0.1222 + legLength;

/// Length of the upper arm in x/y dimensions, when at zero
/// position (pointing outwards at 45 degrees.)
const double diagArmLen = cos(M_PI/4) * 0.060;

shared_ptr<BodyModel const> bodyModel = make_shared<DarwinBodyModel const>();

TEST (BodyStateTests, posture_zeroed)
{
  // All hinges at an angle of zero

  array<double,23> angles;
  angles.fill(0);

  array<short, 21> diffs = std::array<short,21>();
  diffs.fill(0);

  auto body = BodyState(bodyModel, angles, diffs, 1);
  auto leftFoot = body.getLimb("left-foot");
  auto rightFoot = body.getLimb("right-foot");

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-footSide, footForward, -footDownward),
             Vector3d(leftFoot->getTransform().translation()) ));
  EXPECT_TRUE( (leftFoot->getTransform().rotation() - Matrix3d::Identity()).isZero() ) << "Has no rotation";

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-footSide, footForward, -footDownward),
             leftFoot->getTransform() * Vector3d(0,0,0) ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(footSide, -footForward, footDownward),
             leftFoot->getTransform().inverse() * Vector3d(0,0,0) ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(footSide, footForward, -footDownward),
             Vector3d(rightFoot->getTransform().translation()) ));

  EXPECT_TRUE( (rightFoot->getTransform().rotation() - Matrix3d::Identity()).isZero() ) << "Has no rotation";

  // Neck

  EXPECT_TRUE(VectorsEqual(
             Vector3d( 0, 0, 0.0505),
             body.getJoint(JointId::HEAD_PAN)->getPosition() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d( 0, 0, 0.0505),
             body.getJoint(JointId::HEAD_TILT)->getPosition() ));

  // Left arm

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-0.082, 0, 0),
             body.getJoint(JointId::L_SHOULDER_PITCH)->getPosition() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-0.082, 0, -0.016),
             body.getJoint(JointId::L_SHOULDER_ROLL)->getPosition() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-0.082 - diagArmLen, 0.016, -0.016 - diagArmLen),
             body.getJoint(JointId::L_ELBOW)->getPosition() ));

  // Right arm

  EXPECT_TRUE(VectorsEqual(
             Vector3d(0.082, 0, 0),
             body.getJoint(JointId::R_SHOULDER_PITCH)->getPosition() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(0.082, 0, -0.016),
             body.getJoint(JointId::R_SHOULDER_ROLL)->getPosition() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(0.082 + diagArmLen, 0.016, -0.016 - diagArmLen),
             body.getJoint(JointId::R_ELBOW)->getPosition() ));

  // Misc distances

  EXPECT_TRUE(VectorsEqual(
             Vector3d(diagArmLen, 0.016, -diagArmLen),
             BodyState::distanceBetween(
               body.getJoint(JointId::R_SHOULDER_ROLL),
               body.getJoint(JointId::R_ELBOW)) ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(2*0.082 + 2*diagArmLen, 0, 0),
             BodyState::distanceBetween(
               body.getJoint(JointId::L_ELBOW),
               body.getJoint(JointId::R_ELBOW)) ));

  // Verify all three hip joints are coincident

  EXPECT_TRUE(VectorsEqual(
             Vector3d(0, 0, 0),
             BodyState::distanceBetween(
               body.getJoint(JointId::HEAD_TILT),
               body.getJoint(JointId::HEAD_PAN)) )) << "Coincident joints";

  EXPECT_TRUE(VectorsEqual(
             Vector3d(0, 0, 0),
             BodyState::distanceBetween(
               body.getJoint(JointId::L_HIP_PITCH),
               body.getJoint(JointId::L_HIP_ROLL)) )) << "Coincident joints";

  EXPECT_TRUE(VectorsEqual(
             Vector3d(0, 0, 0),
             BodyState::distanceBetween(
               body.getJoint(JointId::L_HIP_YAW),
               body.getJoint(JointId::L_HIP_ROLL)) )) << "Coincident joints";

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-diagArmLen, 0.016, -diagArmLen),
             BodyState::distanceBetween(
               body.getJoint(JointId::L_SHOULDER_ROLL),
               body.getJoint(JointId::L_ELBOW)) ));

  EXPECT_TRUE(
    VectorsEqual(
      Vector3d( diagArmLen + 0.082 + 0.074/2,
               -0.016 - 0.005,
                diagArmLen + 0.016 - 0.1222 - 0.093 - 0.093),
      BodyState::distanceBetween(
        body.getJoint(JointId::L_ELBOW),
        body.getJoint(JointId::R_ANKLE_ROLL)) ));
}

TEST (BodyStateTests, posture_legsToSides)
{
  // Roll the legs out 90 degrees

  array<double,23> angles;
  angles.fill(0);

  angles[(int)JointId::L_HIP_ROLL] = -M_PI/2;
  angles[(int)JointId::R_HIP_ROLL] =  M_PI/2;

  auto body = BodyState(bodyModel, angles, array<short,21>(), 1);
  auto leftFoot = body.getLimb("left-foot");
  auto rightFoot = body.getLimb("right-foot");

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-0.074/2 - legLength, footForward, -0.1222),
             Vector3d(leftFoot ->getTransform().translation()) ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d( 0.074/2 + legLength, footForward, -0.1222),
             Vector3d(rightFoot->getTransform().translation()) ));

  EXPECT_TRUE(MatricesEqual(
             AngleAxisd( M_PI/2, Vector3d::UnitY()).matrix(),
             leftFoot ->getTransform().rotation().matrix() ));

  EXPECT_TRUE(MatricesEqual(
             AngleAxisd(-M_PI/2, Vector3d::UnitY()).matrix(),
             rightFoot->getTransform().rotation().matrix() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(4*0.093 + 0.074, 0, 0),
             BodyState::distanceBetween(
               body.getJoint(JointId::L_ANKLE_PITCH),
               body.getJoint(JointId::R_ANKLE_PITCH)) ));
}

TEST (BodyStateTests, posture_legsForwards)
{
  // Pitch the legs forwards 90 degrees

  array<double,23> angles;
  angles.fill(0);
  angles[(int)JointId::L_HIP_PITCH] =  M_PI/2;
  angles[(int)JointId::R_HIP_PITCH] = -M_PI/2;

  auto body = BodyState(bodyModel, angles, array<short,21>(), 1);
  auto leftFoot = body.getLimb("left-foot");
  auto rightFoot = body.getLimb("right-foot");

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-0.074/2, footForward + legLength, -0.1222),
             Vector3d(leftFoot ->getTransform().translation()) ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d( 0.074/2, footForward + legLength, -0.1222),
             Vector3d(rightFoot->getTransform().translation()) ));

  EXPECT_TRUE(MatricesEqual(
             AngleAxisd(M_PI/2, Vector3d::UnitX()).matrix(),
             leftFoot ->getTransform().rotation().matrix() ));

  EXPECT_TRUE(MatricesEqual(
             AngleAxisd(M_PI/2, Vector3d::UnitX()).matrix(),
             rightFoot->getTransform().rotation().matrix() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(0.074, 0, 0),
             BodyState::distanceBetween(
               body.getJoint(JointId::L_ANKLE_PITCH),
               body.getJoint(JointId::R_ANKLE_PITCH)) ));
}

TEST (BodyStateTests, posture_kneesBentNinetyDegrees)
{
  // Bend the knees back 90 degrees

  array<double,23> angles;
  angles.fill(0);

  angles[(int)JointId::L_KNEE] = -M_PI/2; // TODO VERIFY ANGLES
  angles[(int)JointId::R_KNEE] =  M_PI/2;

  auto body = BodyState(bodyModel, angles, array<short,21>(), 1);
  auto leftFoot = body.getLimb("left-foot");
  auto rightFoot = body.getLimb("right-foot");

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-0.074/2, footForward, -0.1222 - 0.093),
             body.getJoint(JointId::L_KNEE)->getPosition() ));
  EXPECT_TRUE(VectorsEqual(
             Vector3d( 0.074/2, footForward, -0.1222 - 0.093),
             body.getJoint(JointId::R_KNEE)->getPosition() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-0.074/2, footForward - 0.093, -0.1222 - 0.093),
             body.getJoint(JointId::L_ANKLE_PITCH)->getPosition() ));
  EXPECT_TRUE(VectorsEqual(
             Vector3d( 0.074/2, footForward - 0.093, -0.1222 - 0.093),
             body.getJoint(JointId::R_ANKLE_PITCH)->getPosition() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(-0.074/2, footForward - 0.093 - 0.0335, -0.1222 - 0.093),
             Vector3d(leftFoot ->getTransform().translation()) ));
  EXPECT_TRUE(VectorsEqual(
             Vector3d( 0.074/2, footForward - 0.093 - 0.0335, -0.1222 - 0.093),
             Vector3d(rightFoot->getTransform().translation()) ));

  EXPECT_TRUE(MatricesEqual(
             AngleAxisd(-M_PI/2, Vector3d::UnitX()).matrix(),
             leftFoot ->getTransform().rotation().matrix() ));
  EXPECT_TRUE(MatricesEqual(
             AngleAxisd(-M_PI/2, Vector3d::UnitX()).matrix(),
             rightFoot->getTransform().rotation().matrix() ));

  EXPECT_TRUE(VectorsEqual(
             Vector3d(0.074, 0, 0),
             BodyState::distanceBetween(
               body.getJoint(JointId::L_ANKLE_PITCH),
               body.getJoint(JointId::R_ANKLE_PITCH)) ));
}

TEST (BodyStateTests, camera_zeroed)
{
  array<double,23> angles;
  angles.fill(0);

  angles[(uchar)JointId::CAMERA_CALIB_TILT] = -Math::degToRad(40);

  auto body = BodyState(bodyModel, angles, array<short,21>(), 0);

  Affine3d cameraTransform = body.getLimb("camera")->getTransform();

  Matrix3d expectedCameraRotation = AngleAxisd(-Math::degToRad(40), Vector3d::UnitX()).matrix();
  Matrix3d actualCameraRotation = cameraTransform.rotation().matrix();

  // These values describe the position of the camera in the head frame
  const double newZ = 0.00501138;
  const double newY = 0.0475446;

  EXPECT_TRUE( VectorsEqual(Vector3d(0, newY, 0.0505 + newZ),
                            Vector3d(cameraTransform.translation())) );

  EXPECT_TRUE( MatricesEqual(expectedCameraRotation, actualCameraRotation) );

  const double cameraHeight = 0.0335 + 0.093 + 0.093 + 0.1222 + 0.0505 + newZ;

  EXPECT_TRUE( VectorsEqual(Vector3d(0, newY, cameraHeight),
                            Vector3d(body.getAgentCameraTransform().translation())) );

  Matrix3d actualAgentCameraRotation = body.getAgentCameraTransform().rotation().matrix();

  EXPECT_TRUE( MatricesEqual(expectedCameraRotation, actualAgentCameraRotation) );
}

TEST (BodyStateTests, camera_headTiltedBack)
{
  array<double,23> angles;
  angles.fill(0);

  // When the camera is tilted up 40 degrees, the image plane is parallel with the torso's z-axis.
  angles[(uchar)JointId::HEAD_TILT] = Math::degToRad(40);
  angles[(uchar)JointId::CAMERA_CALIB_TILT] = -Math::degToRad(40);

  auto body = BodyState(bodyModel, angles, array<short,21>(), 0);

  Affine3d cameraTransform = body.getLimb("camera")->getTransform();

  Matrix3d expectedCameraRotation = AngleAxisd(0, Vector3d::UnitX()).matrix();
  Matrix3d actualCameraRotation = cameraTransform.rotation().matrix();

  EXPECT_TRUE( VectorsEqual(Vector3d(0, 0.0332, 0.0505 + 0.0344),
                            Vector3d(cameraTransform.translation())) );

  EXPECT_TRUE( MatricesEqual(expectedCameraRotation, actualCameraRotation) );

  const double cameraHeight = (33.5 + 93 + 93 + 122.2 + 50.5 + 34.4) / 1000.0;

  EXPECT_EQ(0.4266, cameraHeight);

  EXPECT_TRUE( VectorsEqual(Vector3d(0, 0.0332, cameraHeight),
                            Vector3d(body.getAgentCameraTransform().translation())) );

  Matrix3d actualAgentCameraRotation = body.getAgentCameraTransform().rotation().matrix();

  EXPECT_TRUE( MatricesEqual(expectedCameraRotation, actualAgentCameraRotation) );
}

TEST (BodyStateTests, centreOfMass)
{
  array<double,23> angles;
  angles.fill(0);

  auto body = BodyState(bodyModel, angles, array<short,21>(), 1);

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(-0.0031855364470584095, -0.0031468527893573723, -0.10594546407578488),
    body.getCentreOfMass()) );
}

TEST (DISABLED_BodyStateTests, cameraNeckJointTransform)
{
  // TODO
}

TEST (BodyStateTests, torsoHeight)
{
  array<double,23> angles;
  angles.fill(0);

  auto body = BodyState(bodyModel, angles, array<short,21>(), 1);

  EXPECT_EQ ( (33.5 + 93 + 93 + 122.2) / 1000.0, body.getTorsoHeight() );
}

TEST (DISABLED_JointTest, initialState)
{
  Joint joint(JointId::L_KNEE, "test-joint");

  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), joint.axis) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), joint.anchors.first) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), joint.anchors.second) );
  EXPECT_EQ   ( nullptr, joint.childPart );

  auto sharedJoint = allocate_aligned_shared<Joint>(JointId::L_KNEE, "test-limb");

  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), sharedJoint->axis) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), sharedJoint->anchors.first) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), sharedJoint->anchors.second) );
  EXPECT_EQ   ( nullptr, sharedJoint->childPart );
}

TEST (LimbPositionTest, initialState)
{
  auto limb = make_shared<Limb const>("test-limb");
  LimbPosition limbPos(limb, Affine3d::Identity());

  EXPECT_EQ ( limb, limbPos.getLimb() );
//  EXPECT_EQ ( Affine3d::Identity(), limbPos.getTransform() );
}

TEST (LimbTest, initialState)
{
  Limb limb("test-limb");

  EXPECT_EQ ( "test-limb", limb.name );
  EXPECT_EQ ( 0, limb.joints.size() );

  auto sharedLimb = allocate_aligned_shared<Limb>("test-limb");

  EXPECT_EQ ( "test-limb", sharedLimb->name );
  EXPECT_EQ ( 0, sharedLimb->joints.size() );
}

TEST (JointTest, initialState)
{
  Joint joint(JointId::HEAD_PAN, "test-joint");

  EXPECT_EQ ( "test-joint", joint.name );
  EXPECT_EQ ( JointId::HEAD_PAN, joint.id );
  EXPECT_EQ ( 0, joint.rotationOrigin );
  EXPECT_EQ ( nullptr, joint.childPart.get() );

  auto sharedJoint = allocate_aligned_shared<Joint const>(JointId::HEAD_PAN, "test-joint");

  EXPECT_EQ ( "test-joint", sharedJoint->name );
  EXPECT_EQ ( JointId::HEAD_PAN, sharedJoint->id );
  EXPECT_EQ ( 0, sharedJoint->rotationOrigin );
  EXPECT_EQ ( nullptr, sharedJoint->childPart.get() );
}
