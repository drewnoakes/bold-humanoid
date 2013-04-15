#ifndef BOLD_ALARMSTATE_HH
#define BOLD_ALARMSTATE_HH

#include <cassert>
#include <vector>

#include "../stateobject.hh"
#include "../../robotis/Framework/include/JointData.h"
#include "../../MX28Alarm/mx28alarm.hh"

namespace bold
{
  class AlarmState : public StateObject
  {
  public:
    AlarmState(std::vector<MX28Alarm> alarmLedByJointId)
    : d_alarmLedByJointId(alarmLedByJointId)
    {
      assert(alarmLedByJointId.size() == Robot::JointData::NUMBER_OF_JOINTS);
    }

    MX28Alarm const getAlarmLed(unsigned jointId) const
    {
      assert(jointId > 0 && jointId < Robot::JointData::NUMBER_OF_JOINTS);

      return d_alarmLedByJointId[jointId];
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::vector<MX28Alarm> d_alarmLedByJointId;
  };
}

#endif
