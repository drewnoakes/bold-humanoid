#include "agentmodel.ih"

void AgentModel::initBody()
{
  d_torso = make_shared<Limb>();
  d_torso->name = "torso";

  // TODO add gyro / acc joints, as done for camera

  // NECK + HEAD

  auto torsoNeckJoint = make_shared<Joint>();
  torsoNeckJoint->id = JointData::ID_HEAD_PAN;
  torsoNeckJoint->axis = Vector3d(0, 1, 0);
  torsoNeckJoint->anchors.first = Vector3d(0, 0.0505, 0);
  torsoNeckJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(torsoNeckJoint);

  auto neck = make_shared<Limb>();
  neck->name = "neck";
  torsoNeckJoint->bodyPart = neck;

  auto neckHeadJoint = make_shared<Joint>();
  neckHeadJoint->id = JointData::ID_HEAD_TILT;
  neckHeadJoint->axis = Vector3d(-1, 0, 0);
  neckHeadJoint->anchors.first = Vector3d(0, 0, 0);
  neckHeadJoint->anchors.second = Vector3d(0, 0, 0);
  neck->joints.push_back(neckHeadJoint);

  auto head = make_shared<Limb>();
  head->name = "head";
  neckHeadJoint->bodyPart = head;

  auto headCameraJoint = make_shared<Joint>();
  headCameraJoint->id = -1;
  // TODO what to do about the camera's orientation
  headCameraJoint->axis = Vector3d(1, 0, 0);
  headCameraJoint->anchors.first = Vector3d(0, 0, 0);
  headCameraJoint->anchors.second = Vector3d(0, 0, 0);
  neck->joints.push_back(headCameraJoint);

  auto camera = make_shared<Limb>();
  camera->name = "camera";
  headCameraJoint->bodyPart = camera;

  // LEFT ARM

  auto ltorsoShoulderJoint = make_shared<Joint>();
  ltorsoShoulderJoint->id = JointData::ID_L_SHOULDER_PITCH;
  ltorsoShoulderJoint->axis = Vector3d(1, 0, 0);
  ltorsoShoulderJoint->anchors.first = Vector3d(0.082, 0, 0);
  ltorsoShoulderJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(ltorsoShoulderJoint);

  auto lshoulderShoulderJoint = make_shared<Joint>();
  lshoulderShoulderJoint->id = JointData::ID_L_SHOULDER_ROLL;
  lshoulderShoulderJoint->axis = Vector3d(0, 0, -1);
  lshoulderShoulderJoint->anchors.first = Vector3d(0, 0, 0);
  lshoulderShoulderJoint->anchors.second = Vector3d(0, 0.016, 0);
  ltorsoShoulderJoint->bodyPart = lshoulderShoulderJoint;

  auto lupperArm = make_shared<Limb>();
  camera->name = "lUpperArm";
  lshoulderShoulderJoint->bodyPart = lupperArm;

  auto lupperLowerArmJoint = make_shared<Joint>();
  lupperLowerArmJoint->id = JointData::ID_L_ELBOW;
  lupperLowerArmJoint->axis = Vector3d(1, 0, 0);
  lupperLowerArmJoint->anchors.first = Vector3d(0, -0.060, 0.016);
  lupperLowerArmJoint->anchors.second = Vector3d(0, 0, 0);
  lupperArm->joints.push_back(lupperLowerArmJoint);

  auto llowerArm = make_shared<Limb>();
  camera->name = "lLowerArm";
  lupperLowerArmJoint->bodyPart = llowerArm;

  // RIGHT ARM

  auto rtorsoShoulderJoint = make_shared<Joint>();
  rtorsoShoulderJoint->id = JointData::ID_R_SHOULDER_PITCH;
  rtorsoShoulderJoint->axis = Vector3d(-1, 0, 0);
  rtorsoShoulderJoint->anchors.first = Vector3d(-0.082, 0, 0);
  rtorsoShoulderJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(rtorsoShoulderJoint);

  auto rshoulderShoulderJoint = make_shared<Joint>();
  rshoulderShoulderJoint->id = JointData::ID_R_SHOULDER_ROLL;
  rshoulderShoulderJoint->axis = Vector3d(0, 0, -1);
  rshoulderShoulderJoint->anchors.first = Vector3d(0, 0, 0);
  rshoulderShoulderJoint->anchors.second = Vector3d(0, 0.016, 0);
  rtorsoShoulderJoint->bodyPart = rshoulderShoulderJoint;

  auto rupperArm = make_shared<Limb>();
  camera->name = "rUpperArm";
  rshoulderShoulderJoint->bodyPart = rupperArm;

  auto rupperLowerArmJoint = make_shared<Joint>();
  rupperLowerArmJoint->id = JointData::ID_R_ELBOW;
  rupperLowerArmJoint->axis = Vector3d(-1, 0, 0);
  rupperLowerArmJoint->anchors.first = Vector3d(0, -0.060, 0.016);
  rupperLowerArmJoint->anchors.second = Vector3d(0, 0, 0);
  rupperArm->joints.push_back(rupperLowerArmJoint);

  auto rlowerArm = make_shared<Limb>();
  camera->name = "rLowerArm";
  rupperLowerArmJoint->bodyPart = rlowerArm;

  // LEFT LEG

  auto ltorsoHipJoint = make_shared<Joint>();
  ltorsoHipJoint->id = JointData::ID_L_HIP_YAW;
  ltorsoHipJoint->axis = Vector3d(0, -1, 0);
  ltorsoHipJoint->anchors.first = Vector3d(0.037, -0.1222, -0.005);
  ltorsoHipJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(ltorsoHipJoint);

  auto lHipHip1Joint = make_shared<Joint>();
  lHipHip1Joint->id = JointData::ID_L_HIP_ROLL;
  lHipHip1Joint->axis = Vector3d(0, 0, 1);
  lHipHip1Joint->anchors.first = Vector3d(0, 0, 0);
  lHipHip1Joint->anchors.second = Vector3d(0, 0, 0);
  ltorsoHipJoint->bodyPart = lHipHip1Joint;

  auto lHipHip2Joint = make_shared<Joint>();
  lHipHip2Joint->id = JointData::ID_L_HIP_PITCH;
  lHipHip2Joint->axis = Vector3d(-1, 0, 0);
  lHipHip2Joint->anchors.first = Vector3d(0, 0, 0);
  lHipHip2Joint->anchors.second = Vector3d(0, 0, 0);
  lHipHip1Joint->bodyPart = lHipHip2Joint;

  auto lupperLeg = make_shared<Limb>();
  lupperLeg->name = "lUpperLeg";
  lHipHip2Joint->bodyPart = lupperLeg;

  auto lupperLowerLegJoint = make_shared<Joint>();
  lupperLowerLegJoint->id = JointData::ID_L_KNEE;
  lupperLowerLegJoint->axis = Vector3d(-1, 0, 0);
  lupperLowerLegJoint->anchors.first = Vector3d(0, -0.093, 0);
  lupperLowerLegJoint->anchors.second = Vector3d(0, 0, 0);
  lupperLeg->joints.push_back(lupperLowerLegJoint);

  auto llowerLeg = make_shared<Limb>();
  llowerLeg->name = "lLowerLeg";
  lupperLowerLegJoint->bodyPart = llowerLeg;

  auto llowerLegAnkleJoint = make_shared<Joint>();
  llowerLegAnkleJoint->id = JointData::ID_L_ANKLE_PITCH;
  llowerLegAnkleJoint->axis = Vector3d(1, 0, 0);
  llowerLegAnkleJoint->anchors.first = Vector3d(0, -0.093, 0);
  llowerLegAnkleJoint->anchors.second = Vector3d(0, 0, 0);
  llowerLeg->joints.push_back(llowerLegAnkleJoint);

  auto lankleFootJoint = make_shared<Joint>();
  lankleFootJoint->id = JointData::ID_L_ANKLE_ROLL;
  lankleFootJoint->axis = Vector3d(0, 0, 1);
  lankleFootJoint->anchors.first = Vector3d(0, 0, 0);
  lankleFootJoint->anchors.second = Vector3d(0, 0, 0);
  llowerLegAnkleJoint->bodyPart = lankleFootJoint;

  d_limbLeftFoot = make_shared<Limb>();
  d_limbLeftFoot->name = "lFoot";
  lankleFootJoint->bodyPart = d_limbLeftFoot;

  // RIGHT LEG

  auto rtorsoHipJoint = make_shared<Joint>();
  rtorsoHipJoint->id = JointData::ID_R_HIP_YAW;
  rtorsoHipJoint->axis = Vector3d(0, -1, 0);
  rtorsoHipJoint->anchors.first = Vector3d(-0.037, -0.1222, -0.005);
  rtorsoHipJoint->anchors.second = Vector3d(0, 0, 0);
  d_torso->joints.push_back(rtorsoHipJoint);

  auto rHipHip1Joint = make_shared<Joint>();
  rHipHip1Joint->id = JointData::ID_R_HIP_ROLL;
  rHipHip1Joint->axis = Vector3d(0, 0, -1);
  rHipHip1Joint->anchors.first = Vector3d(0, 0, 0);
  rHipHip1Joint->anchors.second = Vector3d(0, 0, 0);
  rtorsoHipJoint->bodyPart = rHipHip1Joint;

  auto rHipHip2Joint = make_shared<Joint>();
  rHipHip2Joint->id = JointData::ID_R_HIP_PITCH;
  rHipHip2Joint->axis = Vector3d(1, 0, 0);
  rHipHip2Joint->anchors.first = Vector3d(0, 0, 0);
  rHipHip2Joint->anchors.second = Vector3d(0, 0, 0);
  rHipHip1Joint->bodyPart = rHipHip2Joint;

  auto rupperLeg = make_shared<Limb>();
  rupperLeg->name = "rUpperLeg";
  rHipHip2Joint->bodyPart = rupperLeg;

  auto rupperLowerLegJoint = make_shared<Joint>();
  rupperLowerLegJoint->id = JointData::ID_R_KNEE;
  rupperLowerLegJoint->axis = Vector3d(1, 0, 0);
  rupperLowerLegJoint->anchors.first = Vector3d(0, -0.093, 0);
  rupperLowerLegJoint->anchors.second = Vector3d(0, 0, 0);
  rupperLeg->joints.push_back(rupperLowerLegJoint);

  auto rlowerLeg = make_shared<Limb>();
  rlowerLeg->name = "rLowerLeg";
  rupperLowerLegJoint->bodyPart = rlowerLeg;

  auto rlowerLegAnkleJoint = make_shared<Joint>();
  rlowerLegAnkleJoint->id = JointData::ID_R_ANKLE_PITCH;
  rlowerLegAnkleJoint->axis = Vector3d(-1, 0, 0);
  rlowerLegAnkleJoint->anchors.first = Vector3d(0, -0.093, 0);
  rlowerLegAnkleJoint->anchors.second = Vector3d(0, 0, 0);
  rlowerLeg->joints.push_back(rlowerLegAnkleJoint);

  auto rankleFootJoint = make_shared<Joint>();
  rankleFootJoint->id = JointData::ID_R_ANKLE_ROLL;
  rankleFootJoint->axis = Vector3d(0, 0, 1);
  rankleFootJoint->anchors.first = Vector3d(0, 0, 0);
  rankleFootJoint->anchors.second = Vector3d(0, 0, 0);
  rlowerLegAnkleJoint->bodyPart = rankleFootJoint;

  auto rfoot = make_shared<Limb>();
  rfoot->name = "rFoot";
  rankleFootJoint->bodyPart = rfoot;
}
