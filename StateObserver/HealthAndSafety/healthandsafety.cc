#include "healthandsafety.hh"

#include "../CM730Snapshot/cm730snapshot.hh"
#include "../Config/config.hh"
#include "../MX28Snapshot/mx28snapshot.hh"

using namespace bold;
using namespace std;

HealthAndSafety::HealthAndSafety(std::shared_ptr<Voice> voice)
: d_voice(voice),
  d_voltageMovingAverage(Config::getStaticValue<int>("health-and-safety.voltage.smoothing-window-size")),
  d_voltageTrigger(
    (float)Config::getStaticValue<double>("health-and-safety.voltage.low-threshold"),
    (float)Config::getStaticValue<double>("health-and-safety.voltage.high-threshold"),
    true),
  d_temperatureThreshold(Config::getStaticValue<int>("health-and-safety.temperature.high-threshold"))
{}

void HealthAndSafety::observeTyped(shared_ptr<HardwareState const> state)
{
  float voltage = d_voltageMovingAverage.next(state->getCM730State()->voltage);

  switch (d_voltageTrigger.next(voltage))
  {
    case SchmittTriggerTransition::High:
    {
      // Was probably plugged into a charger
      if (d_voice)
        d_voice->say("Thank you"); // haha
      log::info("HealthAndSafety::observeTyped") << "Voltage level restored above " << d_voltageTrigger.getHighThreshold() << " volts";
      break;
    }
    case SchmittTriggerTransition::Low:
    {
      if (d_voice)
        d_voice->say("Help. Low voltage warning.");
      log::warning("HealthAndSafety::observeTyped") << "Voltage level dropped below " << d_voltageTrigger.getLowThreshold() << " volts";
      d_lastVoltageWarningTime = Clock::getTimestamp();
      break;
    }
    case SchmittTriggerTransition::None:
    {
      if (!d_voltageTrigger.isHigh() && Clock::getSecondsSince(d_lastVoltageWarningTime) > 15)
      {
        if (d_voice)
          d_voice->say("Help. My voltage is still low.");
        d_lastVoltageWarningTime = Clock::getTimestamp();
      }
      break;
    }
  }

  if (Clock::getSecondsSince(d_lastTemperatureWarningTime) > 15)
  {
    int maxTemperature = 0;
    uchar maxTemperatureJointId = 0;
    for (uchar jointId = (uchar)JointId::MIN; jointId < (uchar)JointId::MAX; jointId++)
    {
      int temperature = state->getMX28State(jointId)->presentTemp;
      if (temperature > maxTemperature)
      {
        maxTemperature = temperature;
        maxTemperatureJointId = jointId;
      }
    }

    if (maxTemperature > d_temperatureThreshold)
    {
      if (d_voice)
        d_voice->say("Temperature threshold breach. It's getting hot in here.");
      log::warning("HealthAndSafety::observeTyped") << "Joint " << (int)maxTemperatureJointId << " has temperature of " << maxTemperature << "°C";
      d_lastTemperatureWarningTime = Clock::getTimestamp();
    }
  }
}
