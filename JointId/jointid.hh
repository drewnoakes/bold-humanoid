#pragma once

#include <string>

namespace bold
{
  typedef unsigned char uchar;

  enum class JointId : unsigned char
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

    CAMERA_TILT      = 21,

    // Some convenience pointers
    ARMS_START = R_SHOULDER_PITCH,
    ARMS_END = L_ELBOW,

    LEGS_START = R_HIP_YAW,
    LEGS_END = L_ANKLE_ROLL,

    HEAD_START = HEAD_PAN,
    HEAD_END = HEAD_TILT,

    MIN = R_SHOULDER_PITCH,
    MAX = HEAD_TILT
  };

  class JointName
  {
  public:
    JointName() = delete;

    static std::string getName(uchar jointId)
    {
      return getName((JointId)jointId);
    }

    static std::string getName(JointId jointId)
    {
      switch (jointId)
      {
        case JointId::R_SHOULDER_PITCH: return "R_SHOULDER_PITCH";
        case JointId::L_SHOULDER_PITCH: return "L_SHOULDER_PITCH";
        case JointId::R_SHOULDER_ROLL:  return "R_SHOULDER_ROLL";
        case JointId::L_SHOULDER_ROLL:  return "L_SHOULDER_ROLL";
        case JointId::R_ELBOW:          return "R_ELBOW";
        case JointId::L_ELBOW:          return "L_ELBOW";
        case JointId::R_HIP_YAW:        return "R_HIP_YAW";
        case JointId::L_HIP_YAW:        return "L_HIP_YAW";
        case JointId::R_HIP_ROLL:       return "R_HIP_ROLL";
        case JointId::L_HIP_ROLL:       return "L_HIP_ROLL";
        case JointId::R_HIP_PITCH:      return "R_HIP_PITCH";
        case JointId::L_HIP_PITCH:      return "L_HIP_PITCH";
        case JointId::R_KNEE:           return "R_KNEE";
        case JointId::L_KNEE:           return "L_KNEE";
        case JointId::R_ANKLE_PITCH:    return "R_ANKLE_PITCH";
        case JointId::L_ANKLE_PITCH:    return "L_ANKLE_PITCH";
        case JointId::R_ANKLE_ROLL:     return "R_ANKLE_ROLL";
        case JointId::L_ANKLE_ROLL:     return "L_ANKLE_ROLL";
        case JointId::HEAD_PAN:         return "HEAD_PAN";
        case JointId::HEAD_TILT:        return "HEAD_TILT";
        default:                        return "Unknown";
      }
    }
  };
}
