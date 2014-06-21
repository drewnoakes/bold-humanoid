#include "bodystate.ih"

void BodyState::initialise(array<double,23> const& angles)
{
  d_torso = allocate_aligned_shared<Limb>("torso");

  // TODO add gyro / acc joints, as done for camera

  // NECK + HEAD

  auto headPanJoint = allocate_aligned_shared<Joint>(JointId::HEAD_PAN, "head-pan");
  headPanJoint->axis = Vector3d(0, 0, 1);
  headPanJoint->anchors.first = Vector3d(0, 0, 0.0505);
  headPanJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(headPanJoint);

  auto neckLimb = allocate_aligned_shared<Limb>("neck");
  headPanJoint->childPart = neckLimb;

  auto headTiltJoint = allocate_aligned_shared<Joint>(JointId::HEAD_TILT, "head-tilt");
  headTiltJoint->axis = Vector3d(1, 0, 0);
  headTiltJoint->anchors.first = Vector3d(0, 0, 0);
  headTiltJoint->anchors.second = Vector3d(0, 0, 0);
  neckLimb->joints.push_back(headTiltJoint);

  auto headLimb = allocate_aligned_shared<Limb>("head");
  headTiltJoint->childPart = headLimb;

  // Camera tilt is not an MX28, but rather an adjustable 'hinge' for the
  // camera's orientation within the head, and may be used for calibration.
  // The tilt angle is set via the "camera.calibration.tilt-angle-degrees" setting
  auto cameraCalibrationTiltJoint = allocate_aligned_shared<Joint>(JointId::CAMERA_CALIB_TILT, "camera-calibration-tilt");
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
  auto cameraCalibrationPanJoint = allocate_aligned_shared<Joint>(JointId::CAMERA_CALIB_PAN, "camera-calibration-pan");
  cameraCalibrationPanJoint->axis = Vector3d(0, 0, 1);
  cameraCalibrationPanJoint->anchors.first = Vector3d(0, 0, 0);
  cameraCalibrationPanJoint->anchors.second = Vector3d(0, 0, 0);
  cameraCalibrationTiltJoint->childPart = cameraCalibrationPanJoint;

  auto camera = allocate_aligned_shared<Limb>("camera");
  cameraCalibrationPanJoint->childPart = camera;

  // LEFT ARM

  auto leftShoulderPitchJoint = allocate_aligned_shared<Joint>(JointId::L_SHOULDER_PITCH, "left-shoulder-pitch");
  leftShoulderPitchJoint->axis = Vector3d(-1, 0, 0);
  leftShoulderPitchJoint->anchors.first = Vector3d(-0.082, 0, 0);
  leftShoulderPitchJoint->anchors.second = Vector3d(0, 0, 0.016);
  d_torso->joints.push_back(leftShoulderPitchJoint);

  auto leftShoulderRollJoint = allocate_aligned_shared<Joint>(JointId::L_SHOULDER_ROLL, "left-shoulder-roll");
  leftShoulderRollJoint->rotationOrigin = -M_PI/4.0;
  leftShoulderRollJoint->axis = Vector3d(0, -1, 0);
  leftShoulderRollJoint->anchors.first = Vector3d(0, 0, 0);
  leftShoulderRollJoint->anchors.second = Vector3d(0, 0, 0);
  leftShoulderPitchJoint->childPart = leftShoulderRollJoint;

  auto leftUpperArmLimb = allocate_aligned_shared<Limb>("left-upper-arm");
  leftShoulderRollJoint->childPart = leftUpperArmLimb;

  auto leftElbowJoint = allocate_aligned_shared<Joint>(JointId::L_ELBOW, "left-elbow");
  leftElbowJoint->rotationOrigin = -M_PI/2.0;
  leftElbowJoint->axis = Vector3d(-1, 0, 0);
  leftElbowJoint->anchors.first = Vector3d(0, 0.016, -0.060);
  leftElbowJoint->anchors.second = Vector3d(0, 0, 0);
  leftUpperArmLimb->joints.push_back(leftElbowJoint);

  auto leftLowerArmLimb = allocate_aligned_shared<Limb>("left-lower-arm");
  leftElbowJoint->childPart = leftLowerArmLimb;

  // RIGHT ARM

  auto rightShoulderPitchJoint = allocate_aligned_shared<Joint>(JointId::R_SHOULDER_PITCH, "right-shoulder-pitch");
  rightShoulderPitchJoint->axis = Vector3d(1, 0, 0);
  rightShoulderPitchJoint->anchors.first = Vector3d(0.082, 0, 0);
  rightShoulderPitchJoint->anchors.second = Vector3d(0, 0, 0.016);
  d_torso->joints.push_back(rightShoulderPitchJoint);

  auto rightShoulderRollJoint = allocate_aligned_shared<Joint>(JointId::R_SHOULDER_ROLL, "right-shoulder-roll");
  rightShoulderRollJoint->rotationOrigin = M_PI/4.0;
  rightShoulderRollJoint->axis = Vector3d(0, -1, 0);
  rightShoulderRollJoint->anchors.first = Vector3d(0, 0, 0);
  rightShoulderRollJoint->anchors.second = Vector3d(0, 0, 0);
  rightShoulderPitchJoint->childPart = rightShoulderRollJoint;

  auto rightUpperArmLimb = allocate_aligned_shared<Limb>("right-upper-arm");
  rightShoulderRollJoint->childPart = rightUpperArmLimb;

  auto rightElbowJoint = allocate_aligned_shared<Joint>(JointId::R_ELBOW, "right-elbow");
  rightElbowJoint->rotationOrigin = M_PI/2.0;
  rightElbowJoint->axis = Vector3d(1, 0, 0);
  rightElbowJoint->anchors.first = Vector3d(0, 0.016, -0.060);
  rightElbowJoint->anchors.second = Vector3d(0, 0, 0);
  rightUpperArmLimb->joints.push_back(rightElbowJoint);

  auto rightLowerArmLimb = allocate_aligned_shared<Limb>("right-lower-arm");
  rightElbowJoint->childPart = rightLowerArmLimb;

  // LEFT LEG

  auto leftHipYawJoint = allocate_aligned_shared<Joint>(JointId::L_HIP_YAW, "left-hip-yaw");
  leftHipYawJoint->axis = Vector3d(0, 0, -1);
  leftHipYawJoint->anchors.first = Vector3d(-0.037, -0.005, -0.1222);
  leftHipYawJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(leftHipYawJoint);

  auto leftHipRollJoint = allocate_aligned_shared<Joint>(JointId::L_HIP_ROLL, "left-hip-roll");
  leftHipRollJoint->axis = Vector3d(0, -1, 0);
  leftHipRollJoint->anchors.first = Vector3d(0, 0, 0);
  leftHipRollJoint->anchors.second = Vector3d(0, 0, 0);
  leftHipYawJoint->childPart = leftHipRollJoint;

  auto leftHipPitchJoint = allocate_aligned_shared<Joint>(JointId::L_HIP_PITCH, "left-hip-pitch");
  leftHipPitchJoint->axis = Vector3d(1, 0, 0);
  leftHipPitchJoint->anchors.first = Vector3d(0, 0, 0);
  leftHipPitchJoint->anchors.second = Vector3d(0, 0, 0);
  leftHipRollJoint->childPart = leftHipPitchJoint;

  auto leftUpperLegLimb = allocate_aligned_shared<Limb>("left-upper-leg");
  leftHipPitchJoint->childPart = leftUpperLegLimb;

  auto leftKneeJoint = allocate_aligned_shared<Joint>(JointId::L_KNEE, "left-knee");
  leftKneeJoint->axis = Vector3d(1, 0, 0);
  leftKneeJoint->anchors.first = Vector3d(0, 0, -0.093);
  leftKneeJoint->anchors.second = Vector3d(0, 0, 0);
  leftUpperLegLimb->joints.push_back(leftKneeJoint);

  auto leftLowerLegLimb = allocate_aligned_shared<Limb>("left-lower-leg");
  leftKneeJoint->childPart = leftLowerLegLimb;

  auto leftAnklePitchJoint = allocate_aligned_shared<Joint>(JointId::L_ANKLE_PITCH, "left-ankle-pitch");
  leftAnklePitchJoint->axis = Vector3d(-1, 0, 0);
  leftAnklePitchJoint->anchors.first = Vector3d(0, 0, -0.093);
  leftAnklePitchJoint->anchors.second = Vector3d(0, 0, 0);
  leftLowerLegLimb->joints.push_back(leftAnklePitchJoint);

  auto leftAngleRollJoint = allocate_aligned_shared<Joint>(JointId::L_ANKLE_ROLL, "left-ankle-roll");
  leftAngleRollJoint->axis = Vector3d(0, 1, 0);
  leftAngleRollJoint->anchors.first = Vector3d(0, 0, 0);
  leftAngleRollJoint->anchors.second = Vector3d(0, 0, 0.0335);
  leftAnklePitchJoint->childPart = leftAngleRollJoint;

  auto leftFootLimb = allocate_aligned_shared<Limb>("left-foot");
  leftAngleRollJoint->childPart = leftFootLimb;

  // RIGHT LEG

  auto rightHipYawJoint = allocate_aligned_shared<Joint>(JointId::R_HIP_YAW, "right-hip-yaw");
  rightHipYawJoint->axis = Vector3d(0, 0, -1);
  rightHipYawJoint->anchors.first = Vector3d(0.037, -0.005, -0.1222);
  rightHipYawJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(rightHipYawJoint);

  auto rightHipRollJoint = allocate_aligned_shared<Joint>(JointId::R_HIP_ROLL, "right-hip-roll");
  rightHipRollJoint->axis = Vector3d(0, -1, 0);
  rightHipRollJoint->anchors.first = Vector3d(0, 0, 0);
  rightHipRollJoint->anchors.second = Vector3d(0, 0, 0);
  rightHipYawJoint->childPart = rightHipRollJoint;

  auto rightHipPitchJoint = allocate_aligned_shared<Joint>(JointId::R_HIP_PITCH, "right-hip-pitch");
  rightHipPitchJoint->axis = Vector3d(-1, 0, 0);
  rightHipPitchJoint->anchors.first = Vector3d(0, 0, 0);
  rightHipPitchJoint->anchors.second = Vector3d(0, 0, 0);
  rightHipRollJoint->childPart = rightHipPitchJoint;

  auto rightUpperLegLimb = allocate_aligned_shared<Limb>("right-upper-leg");
  rightHipPitchJoint->childPart = rightUpperLegLimb;

  auto rightKneeJoint = allocate_aligned_shared<Joint>(JointId::R_KNEE, "right-knee");
  rightKneeJoint->axis = Vector3d(-1, 0, 0);
  rightKneeJoint->anchors.first = Vector3d(0, 0, -0.093);
  rightKneeJoint->anchors.second = Vector3d(0, 0, 0);
  rightUpperLegLimb->joints.push_back(rightKneeJoint);

  auto rightLowerLegLimb = allocate_aligned_shared<Limb>("right-lower-leg");
  rightKneeJoint->childPart = rightLowerLegLimb;

  auto rightAnklePitchJoint = allocate_aligned_shared<Joint>(JointId::R_ANKLE_PITCH, "right-ankle-pitch");
  rightAnklePitchJoint->axis = Vector3d(1, 0, 0);
  rightAnklePitchJoint->anchors.first = Vector3d(0, 0, -0.093);
  rightAnklePitchJoint->anchors.second = Vector3d(0, 0, 0);
  rightLowerLegLimb->joints.push_back(rightAnklePitchJoint);

  auto rankleFootJoint = allocate_aligned_shared<Joint>(JointId::R_ANKLE_ROLL, "right-ankle-roll");
  rankleFootJoint->axis = Vector3d(0, 1, 0);
  rankleFootJoint->anchors.first = Vector3d(0, 0, 0);
  rankleFootJoint->anchors.second = Vector3d(0, 0, 0.0335);
  rightAnklePitchJoint->childPart = rankleFootJoint;

  auto rightFootLimb = allocate_aligned_shared<Limb>("right-foot");
  rankleFootJoint->childPart = rightFootLimb;

  //
  // Build all Joint and Limb transforms
  //

  d_torso->transform.setIdentity();

  list<shared_ptr<BodyPart>> partQueue;
  partQueue.push_back(d_torso);

  while (!partQueue.empty())
  {
    shared_ptr<BodyPart> part = partQueue.front();
    partQueue.pop_front();

    if (shared_ptr<Limb> limb = dynamic_pointer_cast<Limb>(part))
    {
      // Limb: Determine transformation of all connected joints by applying
      // proper translation.

      d_limbByName[limb->name] = limb;

      // Loop over all joints extending outwards from this limb
      for (auto joint : limb->joints)
      {
        // Transformation = that of limb plus translation to the joint
        joint->transform
          = limb->transform
          * Translation3d(joint->anchors.first);

        partQueue.push_back(joint);
      }
    }
    else
    {
      // Joint: Determine transformation of joint, apply rotation, then
      // subsequent translation to child part.

      shared_ptr<Joint> joint = dynamic_pointer_cast<Joint>(part);

      ASSERT(joint);

      d_jointById[(uchar)joint->id] = joint;

      joint->angleRads = angles[(uchar)joint->id];

      joint->childPart->transform
        = joint->transform
        * AngleAxisd(joint->rotationOrigin + joint->angleRads, joint->axis)
        * Translation3d(-joint->anchors.second);

      partQueue.push_back(joint->childPart);
    }
  }

  //
  // Other bits
  //

  double zl = leftFootLimb->transform.translation().z();
  double zr = rightFootLimb->transform.translation().z();
  auto lowestFoot = zl < zr ? leftFootLimb : rightFootLimb;

  d_torsoHeight = -std::min(zl, zr);

  // TODO this rotation includes any yaw from the leg which isn't wanted here
  Affine3d lowestFootTorsoRot{lowestFoot->transform.inverse().rotation()};
  Affine3d const& torsoCameraTr = camera->transform;

  // This is a special transform that gives the position of the camera in
  // the agent's frame, taking any rotation of the torso into account
  // considering the orientation of the foot (which is assumed to be flat.)
  d_agentCameraTr = Translation3d(0, 0, d_torsoHeight) * lowestFootTorsoRot * torsoCameraTr;

  // We use the inverse a lot, so calculate it once here.
  d_cameraAgentTr = d_agentCameraTr.inverse();
}
