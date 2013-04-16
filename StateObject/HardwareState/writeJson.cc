#include "hardwarestate.hh"
#include "../CM730Snapshot/cm730snapshot.hh"

using namespace bold;
using namespace rapidjson;

void HardwareState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
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
  }
  writer.EndObject();
}