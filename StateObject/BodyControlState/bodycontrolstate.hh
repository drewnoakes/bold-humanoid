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
      uchar gainP;
      uchar gainI;
      uchar gainD;
    };

    BodyControlState(std::shared_ptr<BodyControl> bodyControl)
    {
      for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
      {
        auto joint = bodyControl->getJoint((JointId)j);

        // TODO consistent capitalisation
        d_jointStates[j - 1].value = joint->getValue();
        d_jointStates[j - 1].gainP = joint->getPGain();
        d_jointStates[j - 1].gainI = joint->getIGain();
        d_jointStates[j - 1].gainD = joint->getDGain();
      }
    };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    JointControlState d_jointStates[(uchar)JointId::MAX];
  };
}
