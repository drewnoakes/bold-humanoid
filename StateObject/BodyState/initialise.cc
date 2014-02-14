#include "bodystate.ih"

void BodyState::initialise(double angles[22])
{
  d_torso = allocate_aligned_shared<Limb>("torso");

  // TODO add gyro / acc joints, as done for camera

  // NECK + HEAD

  auto torsoNeckJoint = allocate_aligned_shared<Joint>(JointId::HEAD_PAN, "head-pan");
  torsoNeckJoint->axis = Vector3d(0, 0, 1);
  torsoNeckJoint->anchors.first = Vector3d(0, 0, 0.0505);
  torsoNeckJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(torsoNeckJoint);

  auto neck = allocate_aligned_shared<Limb>("neck");
  torsoNeckJoint->bodyPart = neck;

  auto neckHeadJoint = allocate_aligned_shared<Joint>(JointId::HEAD_TILT, "head-tilt");
  neckHeadJoint->axis = Vector3d(1, 0, 0);
  neckHeadJoint->anchors.first = Vector3d(0, 0, 0);
  neckHeadJoint->anchors.second = Vector3d(0, 0, 0);
  neck->joints.push_back(neckHeadJoint);

  auto head = allocate_aligned_shared<Limb>("head");
  neckHeadJoint->bodyPart = head;

  auto headCameraJoint = allocate_aligned_shared<Joint>(JointId::CAMERA_TILT, "head-camera");
  // Set angle offset of head here. If this needs to be set for more
  // stuff, probably best to add another joint before this one and set
  // it there
  headCameraJoint->axis = Vector3d(1, 0, 0);
  //headCameraJoint->angleRads = -0.7854;
  headCameraJoint->anchors.first = Vector3d(0, 0, 0);
  headCameraJoint->anchors.second = Vector3d(0, -0.0332, -0.0344);
  head->joints.push_back(headCameraJoint);

  auto camera = allocate_aligned_shared<Limb>("camera");
  headCameraJoint->bodyPart = camera;

  // LEFT ARM

  auto ltorsoShoulderJoint = allocate_aligned_shared<Joint>(JointId::L_SHOULDER_PITCH, "left-shoulder-pitch");
  ltorsoShoulderJoint->axis = Vector3d(-1, 0, 0);
  ltorsoShoulderJoint->anchors.first = Vector3d(-0.082, 0, 0);
  ltorsoShoulderJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(ltorsoShoulderJoint);

  auto lshoulderShoulderJoint = allocate_aligned_shared<Joint>(JointId::L_SHOULDER_ROLL, "left-shoulder-roll");
  lshoulderShoulderJoint->rotationOrigin = -M_PI/4.0;
  lshoulderShoulderJoint->axis = Vector3d(0, -1, 0);
  lshoulderShoulderJoint->anchors.first = Vector3d(0, 0, 0);
  lshoulderShoulderJoint->anchors.second = Vector3d(0, 0, 0.016);
  ltorsoShoulderJoint->bodyPart = lshoulderShoulderJoint;

  auto lupperArm = allocate_aligned_shared<Limb>("lUpperArm");
  lshoulderShoulderJoint->bodyPart = lupperArm;

  auto lupperLowerArmJoint = allocate_aligned_shared<Joint>(JointId::L_ELBOW, "left-elbow");
  lupperLowerArmJoint->rotationOrigin = -M_PI/2.0;
  lupperLowerArmJoint->axis = Vector3d(-1, 0, 0);
  lupperLowerArmJoint->anchors.first = Vector3d(0, 0.016, -0.060);
  lupperLowerArmJoint->anchors.second = Vector3d(0, 0, 0);
  lupperArm->joints.push_back(lupperLowerArmJoint);

  auto llowerArm = allocate_aligned_shared<Limb>("lLowerArm");
  lupperLowerArmJoint->bodyPart = llowerArm;

  // RIGHT ARM

  auto rtorsoShoulderJoint = allocate_aligned_shared<Joint>(JointId::R_SHOULDER_PITCH, "right-shoulder-pitch");
  rtorsoShoulderJoint->axis = Vector3d(1, 0, 0);
  rtorsoShoulderJoint->anchors.first = Vector3d(0.082, 0, 0);
  rtorsoShoulderJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(rtorsoShoulderJoint);

  auto rshoulderShoulderJoint = allocate_aligned_shared<Joint>(JointId::R_SHOULDER_ROLL, "right-shoulder-roll");
  rshoulderShoulderJoint->rotationOrigin = M_PI/4.0;
  rshoulderShoulderJoint->axis = Vector3d(0, -1, 0);
  rshoulderShoulderJoint->anchors.first = Vector3d(0, 0, 0);
  rshoulderShoulderJoint->anchors.second = Vector3d(0, 0, 0.016);
  rtorsoShoulderJoint->bodyPart = rshoulderShoulderJoint;

  auto rupperArm = allocate_aligned_shared<Limb>("rUpperArm");
  rshoulderShoulderJoint->bodyPart = rupperArm;

  auto rupperLowerArmJoint = allocate_aligned_shared<Joint>(JointId::R_ELBOW, "right-elbow");
  rupperLowerArmJoint->rotationOrigin = M_PI/2.0;
  rupperLowerArmJoint->axis = Vector3d(1, 0, 0);
  rupperLowerArmJoint->anchors.first = Vector3d(0, 0.016, -0.060);
  rupperLowerArmJoint->anchors.second = Vector3d(0, 0, 0);
  rupperArm->joints.push_back(rupperLowerArmJoint);

  auto rlowerArm = allocate_aligned_shared<Limb>("rLowerArm");
  rupperLowerArmJoint->bodyPart = rlowerArm;

  // LEFT LEG

  auto ltorsoHipJoint = allocate_aligned_shared<Joint>(JointId::L_HIP_YAW, "left-hip-yaw");
  ltorsoHipJoint->axis = Vector3d(0, 0, -1);
  ltorsoHipJoint->anchors.first = Vector3d(-0.037, -0.005, -0.1222);
  ltorsoHipJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(ltorsoHipJoint);

  auto lHipHip1Joint = allocate_aligned_shared<Joint>(JointId::L_HIP_ROLL, "left-hip-roll");
  lHipHip1Joint->axis = Vector3d(0, 1, 0);
  lHipHip1Joint->anchors.first = Vector3d(0, 0, 0);
  lHipHip1Joint->anchors.second = Vector3d(0, 0, 0);
  ltorsoHipJoint->bodyPart = lHipHip1Joint;

  auto lHipHip2Joint = allocate_aligned_shared<Joint>(JointId::L_HIP_PITCH, "left-hip-pitch");
  lHipHip2Joint->axis = Vector3d(1, 0, 0);
  lHipHip2Joint->anchors.first = Vector3d(0, 0, 0);
  lHipHip2Joint->anchors.second = Vector3d(0, 0, 0);
  lHipHip1Joint->bodyPart = lHipHip2Joint;

  auto lupperLeg = allocate_aligned_shared<Limb>("lUpperLeg");
  lHipHip2Joint->bodyPart = lupperLeg;

  auto lupperLowerLegJoint = allocate_aligned_shared<Joint>(JointId::L_KNEE, "left-knee");
  lupperLowerLegJoint->axis = Vector3d(1, 0, 0);
  lupperLowerLegJoint->anchors.first = Vector3d(0, 0, -0.093);
  lupperLowerLegJoint->anchors.second = Vector3d(0, 0, 0);
  lupperLeg->joints.push_back(lupperLowerLegJoint);

  auto llowerLeg = allocate_aligned_shared<Limb>("lLowerLeg");
  lupperLowerLegJoint->bodyPart = llowerLeg;

  auto llowerLegAnkleJoint = allocate_aligned_shared<Joint>(JointId::L_ANKLE_PITCH, "left-ankle-pitch");
  llowerLegAnkleJoint->axis = Vector3d(-1, 0, 0);
  llowerLegAnkleJoint->anchors.first = Vector3d(0, 0, -0.093);
  llowerLegAnkleJoint->anchors.second = Vector3d(0, 0, 0);
  llowerLeg->joints.push_back(llowerLegAnkleJoint);

  auto lankleFootJoint = allocate_aligned_shared<Joint>(JointId::L_ANKLE_ROLL, "left-ankle-roll");
  lankleFootJoint->axis = Vector3d(0, 1, 0);
  lankleFootJoint->anchors.first = Vector3d(0, 0, 0);
  lankleFootJoint->anchors.second = Vector3d(0, 0, 0.0335);
  llowerLegAnkleJoint->bodyPart = lankleFootJoint;

  auto lfoot = allocate_aligned_shared<Limb>("lFoot");
  lankleFootJoint->bodyPart = lfoot;

  // RIGHT LEG

  auto rtorsoHipJoint = allocate_aligned_shared<Joint>(JointId::R_HIP_YAW, "right-hip-yaw");
  rtorsoHipJoint->axis = Vector3d(0, 0, -1);
  rtorsoHipJoint->anchors.first = Vector3d(0.037, -0.005, -0.1222);
  rtorsoHipJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(rtorsoHipJoint);

  auto rHipHip1Joint = allocate_aligned_shared<Joint>(JointId::R_HIP_ROLL, "right-hip-roll");
  rHipHip1Joint->axis = Vector3d(0, -1, 0);
  rHipHip1Joint->anchors.first = Vector3d(0, 0, 0);
  rHipHip1Joint->anchors.second = Vector3d(0, 0, 0);
  rtorsoHipJoint->bodyPart = rHipHip1Joint;

  auto rHipHip2Joint = allocate_aligned_shared<Joint>(JointId::R_HIP_PITCH, "right-hip-pitch");
  rHipHip2Joint->axis = Vector3d(-1, 0, 0);
  rHipHip2Joint->anchors.first = Vector3d(0, 0, 0);
  rHipHip2Joint->anchors.second = Vector3d(0, 0, 0);
  rHipHip1Joint->bodyPart = rHipHip2Joint;

  auto rupperLeg = allocate_aligned_shared<Limb>("rUpperLeg");
  rHipHip2Joint->bodyPart = rupperLeg;

  auto rupperLowerLegJoint = allocate_aligned_shared<Joint>(JointId::R_KNEE, "right-knee");
  rupperLowerLegJoint->axis = Vector3d(-1, 0, 0);
  rupperLowerLegJoint->anchors.first = Vector3d(0, 0, -0.093);
  rupperLowerLegJoint->anchors.second = Vector3d(0, 0, 0);
  rupperLeg->joints.push_back(rupperLowerLegJoint);

  auto rlowerLeg = allocate_aligned_shared<Limb>("rLowerLeg");
  rupperLowerLegJoint->bodyPart = rlowerLeg;

  auto rlowerLegAnkleJoint = allocate_aligned_shared<Joint>(JointId::R_ANKLE_PITCH, "right-ankle-pitch");
  rlowerLegAnkleJoint->axis = Vector3d(1, 0, 0);
  rlowerLegAnkleJoint->anchors.first = Vector3d(0, 0, -0.093);
  rlowerLegAnkleJoint->anchors.second = Vector3d(0, 0, 0);
  rlowerLeg->joints.push_back(rlowerLegAnkleJoint);

  auto rankleFootJoint = allocate_aligned_shared<Joint>(JointId::R_ANKLE_ROLL, "right-ankle-roll");
  rankleFootJoint->axis = Vector3d(0, 1, 0);
  rankleFootJoint->anchors.first = Vector3d(0, 0, 0);
  rankleFootJoint->anchors.second = Vector3d(0, 0, 0.0335);
  rlowerLegAnkleJoint->bodyPart = rankleFootJoint;

  auto rfoot = allocate_aligned_shared<Limb>("rFoot");
  rankleFootJoint->bodyPart = rfoot;

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

      assert(joint);

      d_jointById[(uchar)joint->id] = joint;

      joint->angleRads = angles[(uchar)joint->id];

      joint->bodyPart->transform
        = joint->transform
        * AngleAxisd(joint->rotationOrigin + joint->angleRads, joint->axis)
        * Translation3d(-joint->anchors.second);

      partQueue.push_back(joint->bodyPart);
    }
  }

  //
  // Other bits
  //
  d_torsoHeight = std::max(
    lfoot->transform.inverse().translation().z(),
    rfoot->transform.inverse().translation().z()
  );

  // TODO determine stance foot
  Affine3d const& footTorsoTransform = rfoot->transform;

  Affine3d torsoAgentRotation(footTorsoTransform.inverse().rotation());

  Affine3d const& cameraTorsoTransform = camera->transform;

  // This is a special transform that gives the position of the camera in
  // the agent's frame, taking any rotation of the torso into account
  // considering the orientation of the foot (which is assumed to be flat.)
  d_cameraAgentTransform = Translation3d(0,0,-footTorsoTransform.translation().z()) * torsoAgentRotation * cameraTorsoTransform;
}
