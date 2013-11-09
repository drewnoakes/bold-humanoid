#pragma once

#include <memory>

#include "../BodyControl/bodycontrol.hh"
#include "../JointId/jointid.hh"
#include "../stateobject.hh"

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

  class BodyControlState : public StateObject
  {
  public:
    struct JointControlState
    {
      ushort value;
      uchar pGain;
      uchar iGain;
      uchar dGain;
    };

    BodyControlState(std::shared_ptr<BodyControl> bodyControl, ulong motionCycleNumber)
    : d_motionCycleNumber(motionCycleNumber)
    {
      for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
      {
        auto joint = bodyControl->getJoint((JointId)j);

        d_jointStates[j - 1].value = joint->getValue();
        d_jointStates[j - 1].pGain = joint->getPGain();
        d_jointStates[j - 1].iGain = joint->getIGain();
        d_jointStates[j - 1].dGain = joint->getDGain();
      }
    };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    ulong d_motionCycleNumber;
    JointControlState d_jointStates[(uchar)JointId::MAX];
  };
}
