#pragma once

#include "../stateobject.hh"
#include "../../Balance/balance.hh"

namespace bold
{
  class BalanceState : public StateObject
  {
  public:
    BalanceState(BalanceOffset const& offsets)
      : d_offsets(offsets)
    {}

    BalanceOffset const& offsets() const { return d_offsets; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    BalanceOffset d_offsets;
  };

  template<typename TBuffer>
  inline void BalanceState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("offsets");
      writer.StartObject();
      {
        writer.String("hipRoll");
        writer.Int(d_offsets.hipRoll);
        writer.String("knee");
        writer.Int(d_offsets.knee);
        writer.String("anklePitch");
        writer.Int(d_offsets.anklePitch);
        writer.String("ankleRoll");
        writer.Int(d_offsets.ankleRoll);
      }
      writer.EndObject();
    }
    writer.EndObject();
  }
}
