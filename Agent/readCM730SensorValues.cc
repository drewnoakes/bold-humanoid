#include "agent.ih"
#include "../AgentModel/agentmodel.hh"

void Agent::readCM730SensorValues()
{
  auto& am = AgentModel::getInstance();

  //
  // READ FROM SUB BOARD
  //
  am.cm730State.init(d_CM730);

  //
  // READ FROM EACH JOINT
  //
  for (int jointId = JointData::ID_R_SHOULDER_PITCH; jointId < JointData::NUMBER_OF_JOINTS; jointId++)
  {
//    MX28Snapshot mx82Snapshot(d_CM730, jointId);
    am.mx28States[jointId].init(d_CM730, jointId);
  }

  am.updated();

//   int gx, gy, gz, ax, ay, az;
//   if (d_CM730.ReadWord(CM730::ID_CM, CM730::P_GYRO_X_L, &gx, 0) == CM730::SUCCESS &&
//       d_CM730.ReadWord(CM730::ID_CM, CM730::P_GYRO_Y_L, &gy, 0) == CM730::SUCCESS &&
//       d_CM730.ReadWord(CM730::ID_CM, CM730::P_GYRO_Z_L, &gz, 0) == CM730::SUCCESS &&
//       d_CM730.ReadWord(CM730::ID_CM, CM730::P_ACCEL_X_L, &ax, 0) == CM730::SUCCESS &&
//       d_CM730.ReadWord(CM730::ID_CM, CM730::P_ACCEL_Y_L, &ay, 0) == CM730::SUCCESS &&
//       d_CM730.ReadWord(CM730::ID_CM, CM730::P_ACCEL_Z_L, &az, 0) == CM730::SUCCESS)
//   {
//     Vector3d gyro = Vector3d(gyroValueToRps(gx), gyroValueToRps(gy), gyroValueToRps(gz));
//     Vector3d acc  = Vector3d(accValueToGs(ax), accValueToGs(ay), accValueToGs(az));
//
//     am.update(gyro, acc);
// //    double dr = rps * dtSec;
// //    facingRads += dr;
// //    printf("VAL: %3d RPS: %.9f DR: %.9f R: %.9f PT: %.8f UT: %.8f",
// //           value, rps, dr, facingRads, linux_cm730.GetPacketTime(), linux_cm730.GetUpdateTime());
//   }

}