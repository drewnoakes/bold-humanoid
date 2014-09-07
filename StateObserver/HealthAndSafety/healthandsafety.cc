#include "healthandsafety.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../Config/config.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../Voice/voice.hh"

using namespace bold;
using namespace std;

HealthAndSafety::HealthAndSafety(std::shared_ptr<Voice> voice)
: TypedStateObserver<HardwareState>("Health & Safety", ThreadId::MotionLoop),
  d_voice(voice),
  d_voltageMovingAverage((ushort)Config::getStaticValue<int>("health-and-safety.voltage.smoothing-window-size")),
  d_voltageTrigger(
    (float)Config::getStaticValue<double>("health-and-safety.voltage.low-threshold"),
    (float)Config::getStaticValue<double>("health-and-safety.voltage.high-threshold"),
    true),
  d_temperatureThreshold(Config::getSetting<int>("health-and-safety.temperature.high-threshold"))
{
  ASSERT(d_voice);

  d_lastTempByJoint.fill((uchar)0);

  int tempSmoothingWindowSize = Config::getStaticValue<int>("health-and-safety.temperature.smoothing-window-size");
  for (int i = 0; i <= (int)JointId::MAX; i++)
    d_averageTempByJoint.emplace_back(tempSmoothingWindowSize);
}

void HealthAndSafety::observeTyped(shared_ptr<HardwareState const> const& state, SequentialTimer& timer)
{
  processVoltage(state);
  processTemperature(state);
}

void HealthAndSafety::processVoltage(shared_ptr<HardwareState const> const& state)
{
  float voltage = d_voltageMovingAverage.next(state->getCM730State().voltage);

  switch (d_voltageTrigger.next(voltage))
  {
    case SchmittTriggerTransition::High:
    {
      // Was probably plugged into a charger
      if (d_voice)
        d_voice->sayOneOf({"Thank you", "Ooo yeah."}); // haha
      log::info("HealthAndSafety::observeTyped") << "Voltage level restored above " << d_voltageTrigger.getHighThreshold() << " volts";
      break;
    }
    case SchmittTriggerTransition::Low:
    {
      if (d_voice)
        d_voice->say("Low voltage warning.");
      log::warning("HealthAndSafety::observeTyped") << "Voltage level dropped below " << d_voltageTrigger.getLowThreshold() << " volts";
      d_lastVoltageWarningTime = Clock::getTimestamp();
      break;
    }
    case SchmittTriggerTransition::None:
    {
      if (!d_voltageTrigger.isHigh() && Clock::getSecondsSince(d_lastVoltageWarningTime) > 20)
      {
        if (d_voice)
          d_voice->say("My voltage is still low.");
        d_lastVoltageWarningTime = Clock::getTimestamp();
      }
      break;
    }
  }
}

void HealthAndSafety::processTemperature(shared_ptr<HardwareState const> const& state)
{
  const uchar tempThreshold = (uchar)d_temperatureThreshold->getValue();

  stringstream msg;

  for (uchar jointId = (uchar)JointId::MIN; jointId < (uchar)JointId::MAX; jointId++)
  {
    const uchar presentTemp = state->getMX28State(jointId).presentTemp;
    const uchar temp = (uchar)round(d_averageTempByJoint[jointId].next(presentTemp));
    const uchar lastTemp = d_lastTempByJoint[jointId];

    // Require a movement of two degrees before making an announcement, avoiding noise on transitions between degrees
    if (abs(temp - lastTemp) < 2)
      continue;

    d_lastTempByJoint[jointId] = temp;

    if (temp > lastTemp)
    {
      // Temperature is increasing
      if (temp >= tempThreshold)
      {
        // We are now above the threshold
        if (lastTemp < tempThreshold)
        {
          // First time above the threshold
          msg << "My " << JointName::getNiceName(jointId) << " is " << (int)temp << " degrees. ";
          log::warning("HealthAndSafety::processTemperature") << JointName::getNiceName(jointId) << " (" << (int)jointId << ") is " << (int)temp << "°C";
        }
        else
        {
          // Was previously above threshold, and now increased
          msg << "My " << JointName::getNiceName(jointId) << " has increased to " << (int)temp << " degrees. ";
          log::warning("HealthAndSafety::processTemperature") << JointName::getNiceName(jointId) << " (" << (int)jointId << ") increased to " << (int)temp << "°C";
        }
      }
    }
    else
    {
      // Temperature is decreasing
      ASSERT(temp < lastTemp);

      if (temp >= tempThreshold)
      {
        // We are still above the threshold, but decreased
        msg << "My " << JointName::getNiceName(jointId) << " has decreased to " << (int)temp << " degrees. ";
        log::warning("HealthAndSafety::processTemperature") << JointName::getNiceName(jointId) << " (" << (int)jointId << ") decreased to " << (int)temp << "°C";
      }
      else if (lastTemp >= tempThreshold)
      {
        // Was previously above threshold, and now below
        msg << "My " << JointName::getNiceName(jointId) << " has decreased to " << (int)temp << " degrees, below threshold. ";
        log::info("HealthAndSafety::processTemperature") << JointName::getNiceName(jointId) << " (" << (int)jointId << ") is below threshold again at " << (int)temp << "°C";
      }
    }
  }

  if (msg.tellp() != 0)
    d_voice->say(msg.str());
}
