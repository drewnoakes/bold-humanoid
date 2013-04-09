#include "agentmodel.ih"

void AgentModel::initBody()
{
  d_torso = make_shared<Limb>();
  d_torso->name = "torso";
  d_limbs["torso"] = d_torso;

  // TODO add gyro / acc joints, as done for camera

  // NECK + HEAD

  auto torsoNeckJoint = make_shared<Joint>();
  torsoNeckJoint->id = JointData::ID_HEAD_PAN;
  torsoNeckJoint->axis = Vector3d(0, 0, 1);
  torsoNeckJoint->anchors.first = Vector3d(0, 0, 0.0505);
  torsoNeckJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(torsoNeckJoint);

  auto neck = make_shared<Limb>();
  neck->name = "neck";
  torsoNeckJoint->bodyPart = neck;
  d_limbs[neck->name] = neck;

  auto neckHeadJoint = make_shared<Joint>();
  neckHeadJoint->id = JointData::ID_HEAD_TILT;
  neckHeadJoint->axis = Vector3d(1, 0, 0);
  neckHeadJoint->anchors.first = Vector3d(0, 0, 0);
  neckHeadJoint->anchors.second = Vector3d(0, 0, 0);
  neck->joints.push_back(neckHeadJoint);

  auto head = make_shared<Limb>();
  head->name = "head";
  neckHeadJoint->bodyPart = head;
  d_limbs[head->name] = head;

  auto headCameraJoint = make_shared<Joint>();
  headCameraJoint->id = -1;
  // Set angle offset of head hear. If this needs to be set for more
  // stuff, probably best to add another joint before this one and set
  // it there
  headCameraJoint->axis = Vector3d(1, 0, 0);
  headCameraJoint->angle = -0.7854;
  headCameraJoint->anchors.first = Vector3d(0, 0, 0);
  headCameraJoint->anchors.second = Vector3d(0, -0.0332, -0.0344);
  head->joints.push_back(headCameraJoint);

  auto camera = make_shared<Limb>();
  camera->name = "camera";
  headCameraJoint->bodyPart = camera;
  d_limbs[camera->name] = camera;

  // LEFT ARM

  auto ltorsoShoulderJoint = make_shared<Joint>();
  ltorsoShoulderJoint->id = JointData::ID_L_SHOULDER_PITCH;
  ltorsoShoulderJoint->axis = Vector3d(-1, 0, 0);
  ltorsoShoulderJoint->anchors.first = Vector3d(-0.082, 0, 0);
  ltorsoShoulderJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(ltorsoShoulderJoint);

  auto lshoulderShoulderJoint = make_shared<Joint>();
  lshoulderShoulderJoint->id = JointData::ID_L_SHOULDER_ROLL;
  lshoulderShoulderJoint->axis = Vector3d(0, -1, 0);
  lshoulderShoulderJoint->anchors.first = Vector3d(0, 0, 0);
  lshoulderShoulderJoint->anchors.second = Vector3d(0, 0, 0.016);
  ltorsoShoulderJoint->bodyPart = lshoulderShoulderJoint;

  auto lupperArm = make_shared<Limb>();
  lupperArm->name = "lUpperArm";
  lshoulderShoulderJoint->bodyPart = lupperArm;
  d_limbs[lupperArm->name] = lupperArm;

  auto lupperLowerArmJoint = make_shared<Joint>();
  lupperLowerArmJoint->id = JointData::ID_L_ELBOW;
  lupperLowerArmJoint->axis = Vector3d(-1, 0, 0);
  lupperLowerArmJoint->anchors.first = Vector3d(0, 0.016, -0.060);
  lupperLowerArmJoint->anchors.second = Vector3d(0, 0, 0);
  lupperArm->joints.push_back(lupperLowerArmJoint);

  auto llowerArm = make_shared<Limb>();
  llowerArm->name = "lLowerArm";
  lupperLowerArmJoint->bodyPart = llowerArm;
  d_limbs[llowerArm->name] = llowerArm;

  // RIGHT ARM

  auto rtorsoShoulderJoint = make_shared<Joint>();
  rtorsoShoulderJoint->id = JointData::ID_R_SHOULDER_PITCH;
  rtorsoShoulderJoint->axis = Vector3d(1, 0, 0);
  rtorsoShoulderJoint->anchors.first = Vector3d(0.082, 0, 0);
  rtorsoShoulderJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(rtorsoShoulderJoint);

  auto rshoulderShoulderJoint = make_shared<Joint>();
  rshoulderShoulderJoint->id = JointData::ID_R_SHOULDER_ROLL;
  rshoulderShoulderJoint->axis = Vector3d(0, -1, 0);
  rshoulderShoulderJoint->anchors.first = Vector3d(0, 0, 0);
  rshoulderShoulderJoint->anchors.second = Vector3d(0, 0, 0.016);
  rtorsoShoulderJoint->bodyPart = rshoulderShoulderJoint;

  auto rupperArm = make_shared<Limb>();
  rupperArm->name = "rUpperArm";
  rshoulderShoulderJoint->bodyPart = rupperArm;
  d_limbs[rupperArm->name] = rupperArm;

  auto rupperLowerArmJoint = make_shared<Joint>();
  rupperLowerArmJoint->id = JointData::ID_R_ELBOW;
  rupperLowerArmJoint->axis = Vector3d(1, 0, 0);
  rupperLowerArmJoint->anchors.first = Vector3d(0, 0.016, -0.060);
  rupperLowerArmJoint->anchors.second = Vector3d(0, 0, 0);
  rupperArm->joints.push_back(rupperLowerArmJoint);

  auto rlowerArm = make_shared<Limb>();
  rlowerArm->name = "rLowerArm";
  rupperLowerArmJoint->bodyPart = rlowerArm;
  d_limbs[rlowerArm->name] = rlowerArm;

  // LEFT LEG

  auto ltorsoHipJoint = make_shared<Joint>();
  ltorsoHipJoint->id = JointData::ID_L_HIP_YAW;
  ltorsoHipJoint->axis = Vector3d(0, 0, -1);
  ltorsoHipJoint->anchors.first = Vector3d(-0.037, -0.005, -0.1222);
  ltorsoHipJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(ltorsoHipJoint);

  auto lHipHip1Joint = make_shared<Joint>();
  lHipHip1Joint->id = JointData::ID_L_HIP_ROLL;
  lHipHip1Joint->axis = Vector3d(0, 1, 0);
  lHipHip1Joint->anchors.first = Vector3d(0, 0, 0);
  lHipHip1Joint->anchors.second = Vector3d(0, 0, 0);
  ltorsoHipJoint->bodyPart = lHipHip1Joint;

  auto lHipHip2Joint = make_shared<Joint>();
  lHipHip2Joint->id = JointData::ID_L_HIP_PITCH;
  lHipHip2Joint->axis = Vector3d(1, 0, 0);
  lHipHip2Joint->anchors.first = Vector3d(0, 0, 0);
  lHipHip2Joint->anchors.second = Vector3d(0, 0, 0);
  lHipHip1Joint->bodyPart = lHipHip2Joint;

  auto lupperLeg = make_shared<Limb>();
  lupperLeg->name = "lUpperLeg";
  lHipHip2Joint->bodyPart = lupperLeg;
  d_limbs[lupperLeg->name] = lupperLeg;

  auto lupperLowerLegJoint = make_shared<Joint>();
  lupperLowerLegJoint->id = JointData::ID_L_KNEE;
  lupperLowerLegJoint->axis = Vector3d(1, 0, 0);
  lupperLowerLegJoint->anchors.first = Vector3d(0, 0, -0.093);
  lupperLowerLegJoint->anchors.second = Vector3d(0, 0, 0);
  lupperLeg->joints.push_back(lupperLowerLegJoint);

  auto llowerLeg = make_shared<Limb>();
  llowerLeg->name = "lLowerLeg";
  lupperLowerLegJoint->bodyPart = llowerLeg;
  d_limbs[llowerLeg->name] = llowerLeg;

  auto llowerLegAnkleJoint = make_shared<Joint>();
  llowerLegAnkleJoint->id = JointData::ID_L_ANKLE_PITCH;
  llowerLegAnkleJoint->axis = Vector3d(1, 0, 0);
  llowerLegAnkleJoint->anchors.first = Vector3d(0, 0, -0.093);
  llowerLegAnkleJoint->anchors.second = Vector3d(0, 0, 0);
  llowerLeg->joints.push_back(llowerLegAnkleJoint);

  auto lankleFootJoint = make_shared<Joint>();
  lankleFootJoint->id = JointData::ID_L_ANKLE_ROLL;
  lankleFootJoint->axis = Vector3d(0, 0, 1);
  lankleFootJoint->anchors.first = Vector3d(0, 0, 0);
  lankleFootJoint->anchors.second = Vector3d(0, 0, 0.0335);
  llowerLegAnkleJoint->bodyPart = lankleFootJoint;

  auto leftFoot = make_shared<Limb>();
  leftFoot->name = "lFoot";
  lankleFootJoint->bodyPart = leftFoot;
  d_limbs[leftFoot->name] = leftFoot;

  // RIGHT LEG

  auto rtorsoHipJoint = make_shared<Joint>();
  rtorsoHipJoint->id = JointData::ID_R_HIP_YAW;
  rtorsoHipJoint->axis = Vector3d(0, 0, -1);
  rtorsoHipJoint->anchors.first = Vector3d(0.037, -0.005, -0.1222);
  rtorsoHipJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(rtorsoHipJoint);

  auto rHipHip1Joint = make_shared<Joint>();
  rHipHip1Joint->id = JointData::ID_R_HIP_ROLL;
  rHipHip1Joint->axis = Vector3d(0, -1, 0);
  rHipHip1Joint->anchors.first = Vector3d(0, 0, 0);
  rHipHip1Joint->anchors.second = Vector3d(0, 0, 0);
  rtorsoHipJoint->bodyPart = rHipHip1Joint;

  auto rHipHip2Joint = make_shared<Joint>();
  rHipHip2Joint->id = JointData::ID_R_HIP_PITCH;
  rHipHip2Joint->axis = Vector3d(-1, 0, 0);
  rHipHip2Joint->anchors.first = Vector3d(0, 0, 0);
  rHipHip2Joint->anchors.second = Vector3d(0, 0, 0);
  rHipHip1Joint->bodyPart = rHipHip2Joint;

  auto rupperLeg = make_shared<Limb>();
  rupperLeg->name = "rUpperLeg";
  rHipHip2Joint->bodyPart = rupperLeg;
  d_limbs[rupperLeg->name] = rupperLeg;

  auto rupperLowerLegJoint = make_shared<Joint>();
  rupperLowerLegJoint->id = JointData::ID_R_KNEE;
  rupperLowerLegJoint->axis = Vector3d(-1, 0, 0);
  rupperLowerLegJoint->anchors.first = Vector3d(0, 0, -0.093);
  rupperLowerLegJoint->anchors.second = Vector3d(0, 0, 0);
  rupperLeg->joints.push_back(rupperLowerLegJoint);

  auto rlowerLeg = make_shared<Limb>();
  rlowerLeg->name = "rLowerLeg";
  rupperLowerLegJoint->bodyPart = rlowerLeg;
  d_limbs[rlowerLeg->name] = rlowerLeg;

  auto rlowerLegAnkleJoint = make_shared<Joint>();
  rlowerLegAnkleJoint->id = JointData::ID_R_ANKLE_PITCH;
  rlowerLegAnkleJoint->axis = Vector3d(1, 0, 0);
  rlowerLegAnkleJoint->anchors.first = Vector3d(0, 0, -0.093);
  rlowerLegAnkleJoint->anchors.second = Vector3d(0, 0, 0);
  rlowerLeg->joints.push_back(rlowerLegAnkleJoint);

  auto rankleFootJoint = make_shared<Joint>();
  rankleFootJoint->id = JointData::ID_R_ANKLE_ROLL;
  rankleFootJoint->axis = Vector3d(0, 1, 0);
  rankleFootJoint->anchors.first = Vector3d(0, 0, 0);
  rankleFootJoint->anchors.second = Vector3d(0, 0, 0.0335);
  rlowerLegAnkleJoint->bodyPart = rankleFootJoint;

  auto rfoot = make_shared<Limb>();
  rfoot->name = "rFoot";
  rankleFootJoint->bodyPart = rfoot;
  d_limbs[rfoot->name] = rfoot;
}
