#include "darwinbodymodel.hh"

#include "../../util/log.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

DarwinBodyModel::DarwinBodyModel()
: BodyModel()
{
  auto torso = createLimb("torso");
  torso->com = Vector3d(-0.0095058847, -0.00050287706, -0.025995316);
  torso->mass = 0.97559947;

  // TODO add gyro / acc joints, as done for camera

  // NECK + HEAD

  auto headPanJoint = createJoint(JointId::HEAD_PAN, "head-pan");
  headPanJoint->axis = Vector3d(0, 0, 1);
  headPanJoint->anchors.first = Vector3d(0, 0, 0.0505);
  headPanJoint->anchors.second = Vector3d(0, 0, 0);
  torso->joints.push_back(headPanJoint);

  auto neckLimb = createLimb("neck");
  neckLimb->mass = 0.024357719;
  neckLimb->com = Vector3d(-0.0014242818, -0.00071281068, -0.016567586);
  headPanJoint->childPart = neckLimb;

  auto headTiltJoint = createJoint(JointId::HEAD_TILT, "head-tilt");
  headTiltJoint->axis = Vector3d(1, 0, 0);
  headTiltJoint->anchors.first = Vector3d(0, 0, 0);
  headTiltJoint->anchors.second = Vector3d(0, 0, 0);
  neckLimb->joints.push_back(headTiltJoint);

  auto headLimb = createLimb("head");
  headLimb->mass = 0.15804192;
  headLimb->com = Vector3d(-0.000063919849, 0.0076666217, 0.018564541);
  headTiltJoint->childPart = headLimb;

  // Camera tilt is not an MX28, but rather an adjustable 'hinge' for the
  // camera's orientation within the head, and may be used for calibration.
  // The tilt angle is set via the "camera.calibration.tilt-angle-degrees" setting
  auto cameraCalibrationTiltJoint = createJoint(JointId::CAMERA_CALIB_TILT, "camera-calibration-tilt");
  cameraCalibrationTiltJoint->axis = Vector3d(1, 0, 0);
  // Camera position is defined in PDF docs when head is tilted up 40 degrees.
  // We need to rotate this around to get the position in the actual frame of
  // the head.
  const double distFromNext = sqrt(pow(0.0332, 2.0) + pow(0.0344, 2.0)); // 0.0478079
  const double theta = atan2(0.0344, 0.0332); // Z==y,Y==x               // 0.803148
  const double adjustedTheta = theta - Math::degToRad(40);               // 0.105016
  const double newZ = distFromNext * sin(adjustedTheta);                 // 0.00501138
  const double newY = distFromNext * cos(adjustedTheta);                 // 0.0475446
  cameraCalibrationTiltJoint->anchors.first = Vector3d(0, newY, newZ);
  cameraCalibrationTiltJoint->anchors.second = Vector3d(0, 0, 0);
  headLimb->joints.push_back(cameraCalibrationTiltJoint);

  // The pan angle is set via the "camera.calibration.pan-angle-degrees" setting
  auto cameraCalibrationPanJoint = createJoint(JointId::CAMERA_CALIB_PAN, "camera-calibration-pan");
  cameraCalibrationPanJoint->axis = Vector3d(0, 0, 1);
  cameraCalibrationPanJoint->anchors.first = Vector3d(0, 0, 0);
  cameraCalibrationPanJoint->anchors.second = Vector3d(0, 0, 0);
  cameraCalibrationTiltJoint->childPart = cameraCalibrationPanJoint;

  auto cameraLimb = createLimb("camera");
  cameraLimb->mass = 0; // included in head limb
  cameraLimb->com = Vector3d(0, 0, 0);
  cameraCalibrationPanJoint->childPart = cameraLimb;

  // LEFT ARM

  auto leftShoulderPitchJoint = createJoint(JointId::L_SHOULDER_PITCH, "left-shoulder-pitch");
  leftShoulderPitchJoint->axis = Vector3d(-1, 0, 0);
  leftShoulderPitchJoint->anchors.first = Vector3d(-0.082, 0, 0);
  leftShoulderPitchJoint->anchors.second = Vector3d(0, 0, 0.016);
  torso->joints.push_back(leftShoulderPitchJoint);

  auto leftShoulderBracketLimb = createLimb("left-shoulder-bracket");
  leftShoulderBracketLimb->mass = 0.025913024;
  leftShoulderBracketLimb->com = Vector3d(0.013522619, 0.0013935747, 0.010264050);
  leftShoulderPitchJoint->childPart = leftShoulderBracketLimb;

  auto leftShoulderRollJoint = createJoint(JointId::L_SHOULDER_ROLL, "left-shoulder-roll");
  leftShoulderRollJoint->rotationOrigin = -M_PI/4.0;
  leftShoulderRollJoint->axis = Vector3d(0, -1, 0);
  leftShoulderRollJoint->anchors.first = Vector3d(0, 0, 0);
  leftShoulderRollJoint->anchors.second = Vector3d(0, 0, 0);
  leftShoulderBracketLimb->joints.push_back(leftShoulderRollJoint);

  auto leftUpperArmLimb = createLimb("left-upper-arm");
  leftUpperArmLimb->mass = 0.16837715;
  leftUpperArmLimb->com = Vector3d(-0.00065978663, 0.00073406497, -0.036239046);
  leftShoulderRollJoint->childPart = leftUpperArmLimb;

  auto leftElbowJoint = createJoint(JointId::L_ELBOW, "left-elbow");
  leftElbowJoint->rotationOrigin = -M_PI/2.0;
  leftElbowJoint->axis = Vector3d(-1, 0, 0);
  leftElbowJoint->anchors.first = Vector3d(0, 0.016, -0.060);
  leftElbowJoint->anchors.second = Vector3d(0, 0, 0);
  leftUpperArmLimb->joints.push_back(leftElbowJoint);

  auto leftLowerArmLimb = createLimb("left-lower-arm");
  leftLowerArmLimb->mass = 0.059288504;
  leftLowerArmLimb->com = Vector3d(-0.0066656445, -0.013490080, -0.045838199);
  leftElbowJoint->childPart = leftLowerArmLimb;

  // RIGHT ARM

  auto rightShoulderPitchJoint = createJoint(JointId::R_SHOULDER_PITCH, "right-shoulder-pitch");
  rightShoulderPitchJoint->axis = Vector3d(1, 0, 0);
  rightShoulderPitchJoint->anchors.first = Vector3d(0.082, 0, 0);
  rightShoulderPitchJoint->anchors.second = Vector3d(0, 0, 0.016);
  torso->joints.push_back(rightShoulderPitchJoint);

  auto rightShoulderBracketLimb = createLimb("right-shoulder-bracket");
  rightShoulderBracketLimb->mass = 0.025913024;
  rightShoulderBracketLimb->com = Vector3d(-0.013522619, 0.0013935747, 0.010264050);
  rightShoulderPitchJoint->childPart = rightShoulderBracketLimb;

  auto rightShoulderRollJoint = createJoint(JointId::R_SHOULDER_ROLL, "right-shoulder-roll");
  rightShoulderRollJoint->rotationOrigin = M_PI/4.0;
  rightShoulderRollJoint->axis = Vector3d(0, -1, 0);
  rightShoulderRollJoint->anchors.first = Vector3d(0, 0, 0);
  rightShoulderRollJoint->anchors.second = Vector3d(0, 0, 0);
  rightShoulderBracketLimb->joints.push_back(rightShoulderRollJoint);

  auto rightUpperArmLimb = createLimb("right-upper-arm");
  rightUpperArmLimb->mass = 0.16837715;
  rightUpperArmLimb->com = Vector3d(0.00065978663, 0.00073406497, -0.036239046);
  rightShoulderRollJoint->childPart = rightUpperArmLimb;

  auto rightElbowJoint = createJoint(JointId::R_ELBOW, "right-elbow");
  rightElbowJoint->rotationOrigin = M_PI/2.0;
  rightElbowJoint->axis = Vector3d(1, 0, 0);
  rightElbowJoint->anchors.first = Vector3d(0, 0.016, -0.060);
  rightElbowJoint->anchors.second = Vector3d(0, 0, 0);
  rightUpperArmLimb->joints.push_back(rightElbowJoint);

  auto rightLowerArmLimb = createLimb("right-lower-arm");
  rightLowerArmLimb->mass = 0.059288504;
  rightLowerArmLimb->com = Vector3d(0.0066656445, -0.013490080, -0.045838199);
  rightElbowJoint->childPart = rightLowerArmLimb;

  // LEFT LEG

  auto leftHipYawJoint = createJoint(JointId::L_HIP_YAW, "left-hip-yaw");
  leftHipYawJoint->axis = Vector3d(0, 0, -1);
  leftHipYawJoint->anchors.first = Vector3d(-0.037, -0.005, -0.1222);
  leftHipYawJoint->anchors.second = Vector3d(0, 0, 0);
  torso->joints.push_back(leftHipYawJoint);

  auto leftHipBracketLimb = createLimb("left-hip-bracket");
  leftHipBracketLimb->mass = 0.027069195;
  leftHipBracketLimb->com = Vector3d(0.0, 0.00048013477, 0.018437244);
  leftHipYawJoint->childPart = leftHipBracketLimb;

  auto leftHipRollJoint = createJoint(JointId::L_HIP_ROLL, "left-hip-roll");
  leftHipRollJoint->axis = Vector3d(0, -1, 0);
  leftHipRollJoint->anchors.first = Vector3d(0, 0, 0);
  leftHipRollJoint->anchors.second = Vector3d(0, 0, 0);
  leftHipBracketLimb->joints.push_back(leftHipRollJoint);

  auto leftButtockLimb = createLimb("left-buttock");
  leftButtockLimb->mass = 0.16710792;
  leftButtockLimb->com = Vector3d(-0.000079982774, -0.018242402, -0.013873116);
  leftHipRollJoint->childPart = leftButtockLimb;

  auto leftHipPitchJoint = createJoint(JointId::L_HIP_PITCH, "left-hip-pitch");
  leftHipPitchJoint->axis = Vector3d(1, 0, 0);
  leftHipPitchJoint->anchors.first = Vector3d(0, 0, 0);
  leftHipPitchJoint->anchors.second = Vector3d(0, 0, 0);
  leftButtockLimb->joints.push_back(leftHipPitchJoint);

  auto leftUpperLegLimb = createLimb("left-upper-leg");
  leftUpperLegLimb->mass = 0.11904336;
  leftUpperLegLimb->com = Vector3d(0.00032263469, 0.00069190565, -0.062965549);
  leftHipPitchJoint->childPart = leftUpperLegLimb;

  auto leftKneeJoint = createJoint(JointId::L_KNEE, "left-knee");
  leftKneeJoint->axis = Vector3d(1, 0, 0);
  leftKneeJoint->anchors.first = Vector3d(0, 0, -0.093);
  leftKneeJoint->anchors.second = Vector3d(0, 0, 0);
  leftUpperLegLimb->joints.push_back(leftKneeJoint);

  auto leftLowerLegLimb = createLimb("left-lower-leg");
  leftLowerLegLimb->mass = 0.070309794;
  leftLowerLegLimb->com = Vector3d(0.00059246918, 0.0065476317, 0.053954532);
  leftKneeJoint->childPart = leftLowerLegLimb;

  auto leftAnklePitchJoint = createJoint(JointId::L_ANKLE_PITCH, "left-ankle-pitch");
  leftAnklePitchJoint->axis = Vector3d(-1, 0, 0);
  leftAnklePitchJoint->anchors.first = Vector3d(0, 0, -0.093);
  leftAnklePitchJoint->anchors.second = Vector3d(0, 0, 0);
  leftLowerLegLimb->joints.push_back(leftAnklePitchJoint);

  auto leftAnkleLimb = createLimb("left-ankle");
  leftAnkleLimb->mass = 0.16710792;
  leftAnkleLimb->com = Vector3d(0.00021373151, -0.018536116, 0.013873116);
  leftAnklePitchJoint->childPart = leftAnkleLimb;

  auto leftAngleRollJoint = createJoint(JointId::L_ANKLE_ROLL, "left-ankle-roll");
  leftAngleRollJoint->axis = Vector3d(0, 1, 0);
  leftAngleRollJoint->anchors.first = Vector3d(0, 0, 0);
  leftAngleRollJoint->anchors.second = Vector3d(0, 0, 0.0335);
  leftAnkleLimb->joints.push_back(leftAngleRollJoint);

  auto leftFootLimb = createLimb("left-foot");
  leftFootLimb->mass = 0.079446226;
  leftFootLimb->com = Vector3d(-0.0095058847, -0.00050287706, -0.025995316);
  leftAngleRollJoint->childPart = leftFootLimb;

  // RIGHT LEG

  auto rightHipYawJoint = createJoint(JointId::R_HIP_YAW, "right-hip-yaw");
  rightHipYawJoint->axis = Vector3d(0, 0, -1);
  rightHipYawJoint->anchors.first = Vector3d(0.037, -0.005, -0.1222);
  rightHipYawJoint->anchors.second = Vector3d(0, 0, 0);
  torso->joints.push_back(rightHipYawJoint);

  auto rightHipBracketLimb = createLimb("right-hip-bracket");
  rightHipBracketLimb->mass = 0.027069195;
  rightHipBracketLimb->com = Vector3d(0.0, 0.00048013477, 0.018437244);
  rightHipYawJoint->childPart = rightHipBracketLimb;

  auto rightHipRollJoint = createJoint(JointId::R_HIP_ROLL, "right-hip-roll");
  rightHipRollJoint->axis = Vector3d(0, -1, 0);
  rightHipRollJoint->anchors.first = Vector3d(0, 0, 0);
  rightHipRollJoint->anchors.second = Vector3d(0, 0, 0);
  rightHipBracketLimb->joints.push_back(rightHipRollJoint);

  auto rightButtockLimb = createLimb("right-buttock");
  rightButtockLimb->mass = 0.16710792;
  rightButtockLimb->com = Vector3d(0.000079982774, -0.018242402, -0.013873116);
  rightHipRollJoint->childPart = rightButtockLimb;

  auto rightHipPitchJoint = createJoint(JointId::R_HIP_PITCH, "right-hip-pitch");
  rightHipPitchJoint->axis = Vector3d(-1, 0, 0);
  rightHipPitchJoint->anchors.first = Vector3d(0, 0, 0);
  rightHipPitchJoint->anchors.second = Vector3d(0, 0, 0);
  rightButtockLimb->joints.push_back(rightHipPitchJoint);

  auto rightUpperLegLimb = createLimb("right-upper-leg");
  rightUpperLegLimb->mass = 0.11904336;
  rightUpperLegLimb->com = Vector3d(-0.00032263469, 0.00069190565, -0.062965549);
  rightHipPitchJoint->childPart = rightUpperLegLimb;

  auto rightKneeJoint = createJoint(JointId::R_KNEE, "right-knee");
  rightKneeJoint->axis = Vector3d(-1, 0, 0);
  rightKneeJoint->anchors.first = Vector3d(0, 0, -0.093);
  rightKneeJoint->anchors.second = Vector3d(0, 0, 0);
  rightUpperLegLimb->joints.push_back(rightKneeJoint);

  auto rightLowerLegLimb = createLimb("right-lower-leg");
  rightLowerLegLimb->mass = 0.070309794;
  rightLowerLegLimb->com = Vector3d(-0.00059246918, 0.0065476317, 0.053954532);
  rightKneeJoint->childPart = rightLowerLegLimb;

  auto rightAnklePitchJoint = createJoint(JointId::R_ANKLE_PITCH, "right-ankle-pitch");
  rightAnklePitchJoint->axis = Vector3d(1, 0, 0);
  rightAnklePitchJoint->anchors.first = Vector3d(0, 0, -0.093);
  rightAnklePitchJoint->anchors.second = Vector3d(0, 0, 0);
  rightLowerLegLimb->joints.push_back(rightAnklePitchJoint);

  auto rightAnkleLimb = createLimb("right-ankle");
  rightAnkleLimb->mass = 0.16710792;
  rightAnkleLimb->com = Vector3d(-0.00021373151, -0.018536116, 0.013873116);
  rightAnklePitchJoint->childPart = rightAnkleLimb;

  auto rankleFootJoint = createJoint(JointId::R_ANKLE_ROLL, "right-ankle-roll");
  rankleFootJoint->axis = Vector3d(0, 1, 0);
  rankleFootJoint->anchors.first = Vector3d(0, 0, 0);
  rankleFootJoint->anchors.second = Vector3d(0, 0, 0.0335);
  rightAnkleLimb->joints.push_back(rankleFootJoint);

  auto rightFootLimb = createLimb("right-foot");
  rightFootLimb->mass = 0.079446226;
  rightFootLimb->com = Vector3d(0.0095058847, -0.00050287706, -0.025995316);
  rankleFootJoint->childPart = rightFootLimb;

  d_torso = torso;
}

std::shared_ptr<Limb> DarwinBodyModel::createLimb(std::string name)
{
  auto limb = allocate_aligned_shared<Limb>(name);
  d_limbByName[name] = limb;
  return limb;
}

std::shared_ptr<Joint> DarwinBodyModel::createJoint(JointId jointId, std::string name)
{
  auto limb = allocate_aligned_shared<Joint>(jointId, name);
  d_jointByName[name] = limb;
  d_jointById[(int)jointId] = limb;
  return limb;
}

std::shared_ptr<Joint const> const& DarwinBodyModel::getJoint(JointId jointId) const
{
  return d_jointById.at((unsigned)jointId);
}

std::shared_ptr<Joint const> const& DarwinBodyModel::getJoint(std::string name) const
{
  auto it = d_jointByName.find(name);
  if (it == d_jointByName.end())
  {
    log::error("DarwinBodyModel::getJoint") << "Request for joint with unknown name: " << name;
    throw std::runtime_error("Request for joint with unknown name");
  }
  return it->second;
}

std::shared_ptr<Limb const> const& DarwinBodyModel::getLimb(std::string name) const
{
  auto it = d_limbByName.find(name);
  if (it == d_limbByName.end())
  {
    log::error("DarwinBodyModel::getLimb") << "Request for limb with unknown name: " << name;
    throw std::runtime_error("Request for limb with unknown name");
  }
  return it->second;
}
