#include "statichardwarestate.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"

#include <memory>

using namespace bold;
using namespace rapidjson;
using namespace std;

void StaticHardwareState::writeJson(Writer<StringBuffer>& writer) const
{
  auto writeAlarm = [&writer](string name, MX28Alarm const& alarm)
  {
    writer.String(name.c_str());
    writer.StartArray();
    {
      for (auto const& setName : alarm.getSetNames())
        writer.String(setName.c_str());
    }
    writer.EndArray();
  };

  writer.StartObject();
  {
    writer.String("id");
    writer.Int(d_cm730State->dynamixelId);
    writer.String("baud");
    writer.Int(d_cm730State->baudBPS);
    writer.String("firmwareVersion");
    writer.Int(d_cm730State->firmwareVersion);
    writer.String("modelNumber");
    writer.Int(d_cm730State->modelNumber);
    writer.String("returnDelayTimeMicroSeconds");
    writer.Int(d_cm730State->returnDelayTimeMicroSeconds);
    writer.String("statusRetLevel");
    writer.Int(d_cm730State->statusRetLevel);

    writer.String("joints");
    writer.StartArray();
    for (shared_ptr<StaticMX28State const> mx28 : d_mx28States)
    {
      writer.StartObject();
      {
        writer.String("id");
        writer.Int(mx28->id);
        writer.String("modelNumber");
        writer.Int(mx28->modelNumber);
        writer.String("firmwareVersion");
        writer.Int(mx28->firmwareVersion);
        writer.String("baud");
        writer.Int(mx28->baudBPS);
        writer.String("returnDelayTimeMicroSeconds");
        writer.Int(mx28->returnDelayTimeMicroSeconds);
        writer.String("angleLimitCW");
        writer.Double(mx28->angleLimitCW);
        writer.String("angleLimitCCW");
        writer.Double(mx28->angleLimitCCW);
        writer.String("tempLimitHighCelsius");
        writer.Int(mx28->tempLimitHighCelsius);
        writer.String("voltageLimitLow");
        writer.Double(mx28->voltageLimitLow);
        writer.String("voltageLimitHigh");
        writer.Double(mx28->voltageLimitHigh);
        writer.String("maxTorque");
        writer.Int(mx28->maxTorque);
        writer.String("statusRetLevel");
        writer.Int(mx28->statusRetLevel);
        writeAlarm("alarmLed", mx28->alarmLed);
        writeAlarm("alarmShutdown", mx28->alarmShutdown);
        writer.String("torqueEnable");
        writer.Bool(mx28->torqueEnable);
        writer.String("isEepromLocked");
        writer.Bool(mx28->isEepromLocked);
      }
      writer.EndObject();
    }
    writer.EndArray();
  }
  writer.EndObject();
}
