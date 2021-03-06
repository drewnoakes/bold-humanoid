#pragma once

#include <string>
#include <stdexcept>

#include "../util/assert.hh"

namespace bold
{
  typedef unsigned char uchar;

  enum class JointId : unsigned char
  {
    // -------------------
    // Actual MX28 joint IDs

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

    // -------------------
    // Psuedo joints

    CAMERA_CALIB_TILT = 21,
    CAMERA_CALIB_PAN  = 22,

    // -------------------
    // Some convenience pointers

    ARMS_START = R_SHOULDER_PITCH,
    ARMS_END = L_ELBOW,

    LEGS_START = R_HIP_YAW,
    LEGS_END = L_ANKLE_ROLL,

    HEAD_START = HEAD_PAN,
    HEAD_END = HEAD_TILT,

    DEVICE_COUNT = HEAD_TILT,
    MIN = R_SHOULDER_PITCH,
    MAX = HEAD_TILT
  };

  bool isHeadJoint(JointId id);

  bool isArmJoint(JointId id);

  bool isLegJoint(JointId id);

  class JointName
  {
  public:
    JointName() = delete;

    static std::string getNiceName(uchar jointId)
    {
      return getNiceName((JointId)jointId);
    }

    static std::string getNiceName(JointId jointId)
    {
      switch (jointId)
      {
        case JointId::R_SHOULDER_PITCH: return "right shoulder pitch";
        case JointId::L_SHOULDER_PITCH: return "left shoulder pitch";
        case JointId::R_SHOULDER_ROLL:  return "right shoulder roll";
        case JointId::L_SHOULDER_ROLL:  return "left shoulder roll";
        case JointId::R_ELBOW:          return "right elbow";
        case JointId::L_ELBOW:          return "left elbow";
        case JointId::R_HIP_YAW:        return "right hip yaw";
        case JointId::L_HIP_YAW:        return "left hip yaw";
        case JointId::R_HIP_ROLL:       return "right hip roll";
        case JointId::L_HIP_ROLL:       return "left hip roll";
        case JointId::R_HIP_PITCH:      return "right hip pitch";
        case JointId::L_HIP_PITCH:      return "left hip pitch";
        case JointId::R_KNEE:           return "right knee";
        case JointId::L_KNEE:           return "left knee";
        case JointId::R_ANKLE_PITCH:    return "right ankle pitch";
        case JointId::L_ANKLE_PITCH:    return "left ankle pitch";
        case JointId::R_ANKLE_ROLL:     return "right ankle roll";
        case JointId::L_ANKLE_ROLL:     return "left ankle roll";
        case JointId::HEAD_PAN:         return "head pan";
        case JointId::HEAD_TILT:        return "head tilt";
        default:                        return "unknown";
      }
    }

    static std::string getEnumName(uchar jointId)
    {
      return getEnumName((JointId)jointId);
    }

    static std::string getEnumName(JointId jointId)
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

    static std::string getJsonName(uchar jointId)
    {
      return getJsonName((JointId)jointId);
    }

    static std::string getJsonName(JointId jointId)
    {
      switch (jointId)
      {
        case JointId::R_SHOULDER_PITCH: return "shoulder-pitch-r";
        case JointId::L_SHOULDER_PITCH: return "shoulder-pitch-l";
        case JointId::R_SHOULDER_ROLL:  return "shoulder-roll-r";
        case JointId::L_SHOULDER_ROLL:  return "shoulder-roll-l";
        case JointId::R_ELBOW:          return "elbow-r";
        case JointId::L_ELBOW:          return "elbow-l";
        case JointId::R_HIP_YAW:        return "hip-yaw-r";
        case JointId::L_HIP_YAW:        return "hip-yaw-l";
        case JointId::R_HIP_ROLL:       return "hip-roll-r";
        case JointId::L_HIP_ROLL:       return "hip-roll-l";
        case JointId::R_HIP_PITCH:      return "hip-pitch-r";
        case JointId::L_HIP_PITCH:      return "hip-pitch-l";
        case JointId::R_KNEE:           return "knee-r";
        case JointId::L_KNEE:           return "knee-l";
        case JointId::R_ANKLE_PITCH:    return "ankle-pitch-r";
        case JointId::L_ANKLE_PITCH:    return "ankle-pitch-l";
        case JointId::R_ANKLE_ROLL:     return "ankle-roll-r";
        case JointId::L_ANKLE_ROLL:     return "ankle-roll-l";
        case JointId::HEAD_PAN:         return "head-pan";
        case JointId::HEAD_TILT:        return "head-tilt";
        default:                        return "unknown";
      }
    }

    static std::string getJsonPairName(uchar jointId)
    {
      return getJsonPairName((JointId)jointId);
    }

    static std::string getJsonPairName(JointId jointId)
    {
      switch (jointId)
      {
        case JointId::R_SHOULDER_PITCH: return "shoulder-pitch";
        case JointId::R_SHOULDER_ROLL:  return "shoulder-roll";
        case JointId::R_ELBOW:          return "elbow";
        case JointId::R_HIP_YAW:        return "hip-yaw";
        case JointId::R_HIP_ROLL:       return "hip-roll";
        case JointId::R_HIP_PITCH:      return "hip-pitch";
        case JointId::R_KNEE:           return "knee";
        case JointId::R_ANKLE_PITCH:    return "ankle-pitch";
        case JointId::R_ANKLE_ROLL:     return "ankle-roll";
        default: throw std::runtime_error("Should access joint pair using base ID (of right side, the lower number)");
      }
    }
  };

  class JointPairs
  {
  public:
    JointPairs() = delete;

    static constexpr bool isBase(JointId jointId) { return isBase((uchar)jointId); }
    static constexpr bool isBase(uchar jointId) { return jointId % 2 == 1 && jointId != (uchar)JointId::HEAD_PAN; }

    static JointId getPartner(JointId jointId)
    {
      return (JointId)getPartner((uchar)jointId);
    }

    static uchar getPartner(uchar jointId)
    {
      ASSERT(!isHeadJoint((JointId)jointId));
      return static_cast<uchar>(jointId + (jointId % 2 == 0 ? -1 : 1));
    }
  };
}
