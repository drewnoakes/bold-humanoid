#pragma once

namespace bold
{
  enum class JointId
  {
    R_SHOULDER_PITCH = 1,
    L_SHOULDER_PITCH = 2,
    R_SHOULDER_ROLL  = 3,
    L_SHOULDER_ROLL  = 4,
    R_ELBOW          = 5,
    L_ELBOW          = 6,

    R_HIP_YAW        = 7,
    L_HIP_YAW        = 8,
    R_HIP_ROLL       = 9,
    L_HIP_ROLL       = 10,
    R_HIP_PITCH      = 11,
    L_HIP_PITCH      = 12,
    R_KNEE           = 13,
    L_KNEE           = 14,
    R_ANKLE_PITCH    = 15,
    L_ANKLE_PITCH    = 16,
    R_ANKLE_ROLL     = 17,
    L_ANKLE_ROLL     = 18,

    HEAD_PAN         = 19,
    HEAD_TILT        = 20,

    // Some convenience pointers
    ARMS_START = R_SHOULDER_PITCH,
    ARMS_END = L_ELBOW,

    LEGS_START = R_HIP_YAW,
    LEGS_END = L_ANKLE_ROLL,

    HEAD_START = HEAD_PAN,
    HEAD_END = HEAD_TILT
  };
}