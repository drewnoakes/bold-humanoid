#pragma once

#include <memory>

#include "../stateobject.hh"
#include "../../JointId/jointid.hh"

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
      short modulation;
      uchar pGain;
      uchar iGain;
      uchar dGain;
    };

    BodyControlState(std::shared_ptr<BodyControl> bodyControl, ulong motionCycleNumber);

    JointControlState const& getJoint(JointId jointId) const { return d_jointStates[(uchar)jointId - 1]; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    ulong d_motionCycleNumber;
    JointControlState d_jointStates[(uchar)JointId::MAX];
  };
}
