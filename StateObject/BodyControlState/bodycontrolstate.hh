#pragma once

#include <memory>

#include "../JointId/jointid.hh"
#include "../stateobject.hh"

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

  class BodyControl;

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

    BodyControlState(std::shared_ptr<BodyControl> bodyControl, ulong motionCycleNumber);

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    ulong d_motionCycleNumber;
    JointControlState d_jointStates[(uchar)JointId::MAX];
  };
}
