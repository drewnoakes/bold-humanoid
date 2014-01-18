#include "hardwarestate.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"

#include <memory>

using namespace bold;
using namespace rapidjson;
using namespace std;

void HardwareState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("cycle").Uint64(d_motionCycleNumber);

    writer.String("acc");
    writer.StartArray();
    writer.Double(d_cm730State->acc.x());
    writer.Double(d_cm730State->acc.y());
    writer.Double(d_cm730State->acc.z());
    writer.EndArray();

    writer.String("gyro");
    writer.StartArray();
    writer.Double(d_cm730State->gyro.x());
    writer.Double(d_cm730State->gyro.y());
    writer.Double(d_cm730State->gyro.z());
    writer.EndArray();

    writer.String("eye");
    writer.StartArray();
    writer.Double(d_cm730State->eyeColor.x());
    writer.Double(d_cm730State->eyeColor.y());
    writer.Double(d_cm730State->eyeColor.z());
    writer.EndArray();

    writer.String("forehead");
    writer.StartArray();
    writer.Double(d_cm730State->foreheadColor.x());
    writer.Double(d_cm730State->foreheadColor.y());
    writer.Double(d_cm730State->foreheadColor.z());
    writer.EndArray();

    writer.String("led2").Bool(d_cm730State->isLed2On);
    writer.String("led3").Bool(d_cm730State->isLed3On);
    writer.String("led4").Bool(d_cm730State->isLed4On);

    writer.String("volts").Double(d_cm730State->voltage);

    writer.String("rxBytes").Uint64(d_rxBytes);
    writer.String("txBytes").Uint64(d_txBytes);

    writer.String("joints");
    writer.StartArray();
    for (auto const& mx28 : d_mx28States)
    {
      writer.StartObject();
      {
        writer.String("id").Int(mx28->id);
//       writer.String("movingSpeedRPM").Int(mx28->movingSpeedRPM);
        writer.String("val").Int(mx28->presentPositionValue);
        writer.String("rpm").Int(mx28->presentSpeedRPM);
        writer.String("load").Int(mx28->presentLoad);
        writer.String("temp").Int(mx28->presentTemp);
        writer.String("volts").Int(mx28->presentVoltage);
      }
      writer.EndObject();
    }
    writer.EndArray();
  }
  writer.EndObject();
}
