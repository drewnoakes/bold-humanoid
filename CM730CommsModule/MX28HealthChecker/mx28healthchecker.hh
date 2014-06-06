#pragma once

#include <array>
#include <memory>

#include "../cm730commsmodule.hh"
#include "../../CM730/cm730.hh"
#include "../../JointId/jointid.hh"
#include "../../MX28Alarm/mx28alarm.hh"
#include "../../SequentialTimer/sequentialtimer.hh"

namespace bold
{
  typedef unsigned char uchar;

  class Voice;

  class MX28HealthChecker : public CM730CommsModule
  {
  public:
    MX28HealthChecker(std::shared_ptr<Voice> voice);

    virtual void step(std::unique_ptr<CM730>& cm730, SequentialTimer& t, ulong motionCycleNumber) override;

  private:
    std::array<bool, (uchar)JointId::MAX + 1> d_isTorqueEnabledByJointId;
    std::array<CommResult, (uchar)JointId::MAX + 1> d_commResultByJointId;
    std::array<MX28Alarm, (uchar)JointId::MAX + 1> d_alarmByJointId;
    std::shared_ptr<Voice> d_voice;
    uchar d_jointId = (uchar)JointId::MIN;
  };
}
