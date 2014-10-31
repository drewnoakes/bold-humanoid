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
