#include "alarmstate.hh"

using namespace bold;
using namespace rapidjson;

void AlarmState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
//     writer.String("angles");
//     writer.StartArray();
//     {
//       for (unsigned j = 1; j < Robot::JointData::NUMBER_OF_JOINTS; j++)
//       {
//         writer.Double(d_alarmLedByJointId[j]->angle);
//       }
//     }
//     writer.EndArray();
  }
  writer.EndObject();
}