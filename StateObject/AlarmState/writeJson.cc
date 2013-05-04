#include "alarmstate.hh"

using namespace std;
using namespace bold;
using namespace rapidjson;
using namespace robotis;

void AlarmState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    for (unsigned j = 1; j < JointData::NUMBER_OF_JOINTS; j++)
    {
      MX28Alarm const& alarm = d_alarmLedByJointId[j];
      writer.Int(j);
      writer.StartArray();
      {
        for (string const& name : alarm.getSetNames())
          writer.String(name.c_str());
      }
      writer.EndArray();
    }
  }
  writer.EndObject();
}