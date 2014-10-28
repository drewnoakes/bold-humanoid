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
    writer.String("cycle");
    writer.Uint64(d_motionCycleNumber);

    writer.String("acc");
    writer.StartArray();
    writer.Double(d_cm730State->acc.x(), "%.3f");
    writer.Double(d_cm730State->acc.y(), "%.3f");
    writer.Double(d_cm730State->acc.z(), "%.3f");
    writer.EndArray();

    writer.String("gyro");
    writer.StartArray();
    writer.Double(d_cm730State->gyro.x(), "%.3f");
    writer.Double(d_cm730State->gyro.y(), "%.3f");
    writer.Double(d_cm730State->gyro.z(), "%.3f");
    writer.EndArray();

    writer.String("eye");
    writer.StartArray();
    writer.Double(d_cm730State->eyeColor.x(), "%.3f");
    writer.Double(d_cm730State->eyeColor.y(), "%.3f");
    writer.Double(d_cm730State->eyeColor.z(), "%.3f");
    writer.EndArray();

    writer.String("forehead");
    writer.StartArray();
    writer.Double(d_cm730State->foreheadColor.x(), "%.3f");
    writer.Double(d_cm730State->foreheadColor.y(), "%.3f");
    writer.Double(d_cm730State->foreheadColor.z(), "%.3f");
    writer.EndArray();

    writer.String("led2");
    writer.Bool(d_cm730State->isLed2On);
    writer.String("led3");
    writer.Bool(d_cm730State->isLed3On);
    writer.String("led4");
    writer.Bool(d_cm730State->isLed4On);

    writer.String("volts");
    writer.Double(d_cm730State->voltage, "%.1f");

    writer.String("rxBytes");
    writer.Uint64(d_rxBytes);
    writer.String("txBytes");
    writer.Uint64(d_txBytes);

    writer.String("joints");
    writer.StartArray();
    for (auto const& mx28 : d_mx28States)
    {
      writer.StartObject();
      {
        writer.String("id");
        writer.Int(mx28->id);
//      writer.String("movingSpeedRPM");
//        writer.Int(mx28->movingSpeedRPM);
        writer.String("val");
        writer.Int(mx28->presentPositionValue);
        writer.String("rpm");
        writer.Double(mx28->presentSpeedRPM);
        writer.String("load");
        writer.Double(mx28->presentLoad);
        writer.String("temp");
        writer.Int(mx28->presentTemp);
        writer.String("volts");
        writer.Double(mx28->presentVoltage);
      }
      writer.EndObject();
    }
    writer.EndArray();
  }
  writer.EndObject();
}
