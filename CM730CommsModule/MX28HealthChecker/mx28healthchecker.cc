#include "mx28healthchecker.hh"

#include "../../CM730/cm730.hh"
#include "../../JointId/jointid.hh"
#include "../../MotionModule/motionmodule.hh"
#include "../../MX28/mx28.hh"
#include "../../MX28Alarm/mx28alarm.hh"
#include "../../util/log.hh"
#include "../../Voice/voice.hh"

using namespace bold;
using namespace std;

MX28HealthChecker::MX28HealthChecker(std::shared_ptr<Voice> voice)
: CM730CommsModule("MX28 Health Checker"),
  d_voice(voice),
  d_jointId((uchar)JointId::MIN)
{
  // Initially assume that all joints have torque enabled
  std::fill(begin(d_isTorqueEnabledByJointId), end(d_isTorqueEnabledByJointId), true);
  std::fill(begin(d_commResultByJointId), end(d_commResultByJointId), CommResult::SUCCESS);
  std::fill(begin(d_alarmByJointId), end(d_alarmByJointId), MX28Alarm());
}

void MX28HealthChecker::step(unique_ptr<CM730>& cm730, SequentialTimer& t, ulong motionCycleNumber)
{
  // Roughly four times a second
  if (motionCycleNumber % 32 != 0)
    return;

  uchar isTorqueEnabled;
  MX28Alarm alarm;

  CommResult res
    = d_commResultByJointId[d_jointId]
      = cm730->readByte(d_jointId, MX28Table::TORQUE_ENABLE, &isTorqueEnabled, &alarm);

  if (res == CommResult::SUCCESS)
  {
    d_alarmByJointId[d_jointId] = alarm;

    if (isTorqueEnabled != d_isTorqueEnabledByJointId[d_jointId])
    {
      // Something changed
      d_isTorqueEnabledByJointId[d_jointId] = isTorqueEnabled;

      // Make an announcement
      stringstream message;
      message << "Torque " << (isTorqueEnabled ? "enabled" : "disabled") << " for " << JointName::getNiceName(d_jointId);

      if (alarm.hasError())
        message << " with error " << alarm;

      // Log it
      log::error("MotionLoop::step") << message.str();

      // Say it out loud too
      if (d_voice->queueLength() < 4)
        d_voice->say(message.str());
    }
  }

  // Wrap around to the next joint
  if (++d_jointId > (uchar)JointId::MAX)
    d_jointId = (uchar)JointId::MIN;

  t.timeEvent("Check Joint Health");
}
