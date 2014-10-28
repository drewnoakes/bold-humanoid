#include "bodycontrolstate.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../JointId/jointid.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

BodyControlState::BodyControlState(shared_ptr<BodyControl> bodyControl, ulong motionCycleNumber)
: d_motionCycleNumber(motionCycleNumber)
{
  for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
  {
    auto joint = bodyControl->getJoint((JointId)j);

    d_jointStates[j - 1].value = joint->getValue();
    d_jointStates[j - 1].modulation = joint->getModulationOffset();
    d_jointStates[j - 1].pGain = joint->getPGain();
    d_jointStates[j - 1].iGain = joint->getIGain();
    d_jointStates[j - 1].dGain = joint->getDGain();
  }
}

void BodyControlState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("cycle");
    writer.Uint64(d_motionCycleNumber);

    writer.String("joints");
    writer.StartArray();
    {
      for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
      {
        writer.StartObject();
        {
          writer.String("v");
          writer.Uint(d_jointStates[j - 1].value);
          writer.String("m");
          writer.Int(d_jointStates[j - 1].modulation);
          writer.String("p");
          writer.Uint(d_jointStates[j - 1].pGain);
          writer.String("i");
          writer.Uint(d_jointStates[j - 1].iGain);
          writer.String("d");
          writer.Uint(d_jointStates[j - 1].dGain);
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
