#include "statichardwarestate.hh"

#include "../CM730Snapshot/cm730snapshot.hh"
#include "../MX28Snapshot/mx28snapshot.hh"

#include <memory>

using namespace bold;
using namespace rapidjson;
using namespace std;

void StaticHardwareState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("id").Int(d_cm730State->dynamixelId);
    writer.String("baud").Int(d_cm730State->baudBPS);
    writer.String("firmwareVersion").Int(d_cm730State->firmwareVersion);
    writer.String("modelNumber").Int(d_cm730State->modelNumber);
    writer.String("returnDelayTimeMicroSeconds").Int(d_cm730State->returnDelayTimeMicroSeconds);
    writer.String("statusRetLevel").Int(d_cm730State->statusRetLevel);

    writer.String("joints");
    writer.StartArray();
    for (shared_ptr<StaticMX28State const> mx28 : d_mx28States)
    {
      writer.StartObject();
      {
        writer.String("id").Int(mx28->id);
        writer.String("modelNumber").Int(mx28->modelNumber);
        writer.String("firmwareVersion").Int(mx28->firmwareVersion);
        writer.String("baud").Int(mx28->baudBPS);
        writer.String("returnDelayTimeMicroSeconds").Int(mx28->returnDelayTimeMicroSeconds);
        writer.String("angleLimitCW").Double(mx28->angleLimitCW);
        writer.String("angleLimitCCW").Double(mx28->angleLimitCCW);
        writer.String("tempLimitHighCelcius").Int(mx28->tempLimitHighCelcius);
        writer.String("voltageLimitLow").Double(mx28->voltageLimitLow);
        writer.String("voltageLimitHigh").Double(mx28->voltageLimitHigh);
        writer.String("maxTorque").Int(mx28->maxTorque);
        writer.String("statusRetLevel").Int(mx28->statusRetLevel);
        writer.String("alarmLed").String(mx28->alarmLed.toString().c_str());
        writer.String("alarmShutdown").String(mx28->alarmShutdown.toString().c_str());
        writer.String("torqueEnable").Bool(mx28->torqueEnable);
        writer.String("isEepromLocked").Bool(mx28->isEepromLocked);
      }
      writer.EndObject();
    }
    writer.EndArray();
  }
  writer.EndObject();
}