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

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    ulong d_motionCycleNumber;
    JointControlState d_jointStates[(uchar)JointId::MAX];
  };

  template<typename TBuffer>
  inline void BodyControlState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
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
}
