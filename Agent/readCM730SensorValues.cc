#include "agent.ih"
#include "../AgentModel/agentmodel.hh"

double gyroValueToDps(int value)
{
    // TODO the range in the positive and negative should be slightly different
    // see http://support.robotis.com/en/product/darwin-op/references/reference/hardware_specifications/electronics/sub_controller_(cm-730).htm
    return ((value-512)/512.0)*1600.0;
}

double gyroValueToRps(int value)
{
    // 0 -> -1600 dps
    // 512 -> 0 dps
    // 1023 -> +1600 dps
    double dps = gyroValueToDps(value);
    return dps*M_PI/180.0;
}

double accValueToGs(int value)
{
    // TODO the range in the positive and negative should be slightly different
    // see http://support.robotis.com/en/product/darwin-op/references/reference/hardware_specifications/electronics/sub_controller_(cm-730).htm
    return ((value-512)/512.0)*4.0;
}

void Agent::readCM730SensorValues()
{
  int gx, gy, gz, ax, ay, az;
  if (d_CM730.ReadWord(CM730::ID_CM, CM730::P_GYRO_X_L, &gx, 0) == CM730::SUCCESS &&
      d_CM730.ReadWord(CM730::ID_CM, CM730::P_GYRO_Y_L, &gy, 0) == CM730::SUCCESS &&
      d_CM730.ReadWord(CM730::ID_CM, CM730::P_GYRO_Z_L, &gz, 0) == CM730::SUCCESS &&
      d_CM730.ReadWord(CM730::ID_CM, CM730::P_ACCEL_X_L, &ax, 0) == CM730::SUCCESS &&
      d_CM730.ReadWord(CM730::ID_CM, CM730::P_ACCEL_Y_L, &ay, 0) == CM730::SUCCESS &&
      d_CM730.ReadWord(CM730::ID_CM, CM730::P_ACCEL_Z_L, &az, 0) == CM730::SUCCESS)
  {
    Vector3d gyro = Vector3d(gyroValueToRps(gx), gyroValueToRps(gy), gyroValueToRps(gz));
    Vector3d acc  = Vector3d(accValueToGs(ax), accValueToGs(ay), accValueToGs(az));

    AgentModel::getInstance().updateCM730Data(gyro, acc);
//    double dr = rps * dtSec;
//    facingRads += dr;
//    printf("VAL: %3d RPS: %.9f DR: %.9f R: %.9f PT: %.8f UT: %.8f",
//           value, rps, dr, facingRads, linux_cm730.GetPacketTime(), linux_cm730.GetUpdateTime());
  }

}