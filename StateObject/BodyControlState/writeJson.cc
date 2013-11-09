#include "bodycontrolstate.hh"

#include "../../JointId/jointid.hh"

using namespace bold;
using namespace rapidjson;

void BodyControlState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("cycle").Uint64(d_motionCycleNumber);
    
    writer.String("joints");
    writer.StartArray();
    {
      for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
      {
        writer.StartObject();
        {
          writer.String("v").Uint(d_jointStates[j - 1].value);
          writer.String("p").Uint(d_jointStates[j - 1].pGain);
          writer.String("i").Uint(d_jointStates[j - 1].iGain);
          writer.String("d").Uint(d_jointStates[j - 1].dGain);
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
