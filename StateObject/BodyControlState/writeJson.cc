#include "bodycontrolstate.hh"

#include "../../JointId/jointid.hh"

using namespace bold;
using namespace rapidjson;

void BodyControlState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartArray();
  {
    for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
    {
      writer.StartObject();
      {
        writer.String("v").Uint(d_jointStates[j - 1].value);
        writer.String("p").Uint(d_jointStates[j - 1].gainP);
        writer.String("i").Uint(d_jointStates[j - 1].gainI);
        writer.String("d").Uint(d_jointStates[j - 1].gainD);
      }
      writer.EndObject();
    }
  }
  writer.EndArray();
}
