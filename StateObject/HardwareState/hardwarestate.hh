#pragma once

#include <memory>
#include <vector>

#include "../stateobject.hh"
#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../JointId/jointid.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"
#include "../../util/assert.hh"

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned long ulong;

  class HardwareState : public StateObject
  {
  public:
    HardwareState(std::unique_ptr<CM730Snapshot const> cm730State,
                  std::vector<std::unique_ptr<MX28Snapshot const>> mx28States,
                  ulong rxBytes,
                  ulong txBytes,
                  ulong motionCycleNumber)
    : d_cm730State(std::move(cm730State)),
      d_mx28States(std::move(mx28States)),
      d_rxBytes(rxBytes),
      d_txBytes(txBytes),
      d_motionCycleNumber(motionCycleNumber)
    {
      ASSERT(d_mx28States.size() == 20);
    }

    CM730Snapshot const& getCM730State() const
    {
      return *d_cm730State;
    }

    MX28Snapshot const& getMX28State(uchar jointId) const
    {
      ASSERT(jointId >= (uchar)JointId::MIN && jointId <= (uchar)JointId::MAX);
      ASSERT(d_mx28States.size() >= jointId);

      return *d_mx28States[jointId - 1];
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

    ulong getReceivedBytes() const { return d_rxBytes; }
    ulong getTransmittedBytes() const { return d_txBytes; }
    ulong getMotionCycleNumber() const { return d_motionCycleNumber; }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    std::unique_ptr<CM730Snapshot const> d_cm730State;
    std::vector<std::unique_ptr<MX28Snapshot const>> d_mx28States;
    ulong d_rxBytes;
    ulong d_txBytes;
    ulong d_motionCycleNumber;
  };

  template<typename TBuffer>
  inline void HardwareState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("cycle");
      writer.Uint64(d_motionCycleNumber);

      writer.String("acc");
      writer.StartArray();
      writer.Double(d_cm730State->acc.x(), "%.3f");
      writer.Double(d_cm730State->acc.y(), "%.3f");
      writer.Double(d_cm730State->acc.z(), "%.3f");
      writer.EndArray();

      writer.String("gyro");
      writer.StartArray();
      writer.Double(d_cm730State->gyro.x(), "%.3f");
      writer.Double(d_cm730State->gyro.y(), "%.3f");
      writer.Double(d_cm730State->gyro.z(), "%.3f");
      writer.EndArray();

      writer.String("eye");
      writer.StartArray();
      writer.Double(d_cm730State->eyeColor.x(), "%.3f");
      writer.Double(d_cm730State->eyeColor.y(), "%.3f");
      writer.Double(d_cm730State->eyeColor.z(), "%.3f");
      writer.EndArray();

      writer.String("forehead");
      writer.StartArray();
      writer.Double(d_cm730State->foreheadColor.x(), "%.3f");
      writer.Double(d_cm730State->foreheadColor.y(), "%.3f");
      writer.Double(d_cm730State->foreheadColor.z(), "%.3f");
      writer.EndArray();

      writer.String("led2");
      writer.Bool(d_cm730State->isLed2On);
      writer.String("led3");
      writer.Bool(d_cm730State->isLed3On);
      writer.String("led4");
      writer.Bool(d_cm730State->isLed4On);

      writer.String("volts");
      writer.Double(d_cm730State->voltage, "%.1f");

      writer.String("rxBytes");
      writer.Uint64(d_rxBytes);
      writer.String("txBytes");
      writer.Uint64(d_txBytes);

      writer.String("joints");
      writer.StartArray();
      for (auto const& mx28 : d_mx28States)
      {
        writer.StartObject();
        {
          writer.String("id");
          writer.Int(mx28->id);
//        writer.String("movingSpeedRPM");
//        writer.Int(mx28->movingSpeedRPM);
          writer.String("val");
          writer.Int(mx28->presentPositionValue);
          writer.String("rpm");
          writer.Double(mx28->presentSpeedRPM, "%.3f");
          writer.String("load");
          writer.Double(mx28->presentLoad, "%.3f");
          writer.String("temp");
          writer.Int(mx28->presentTemp);
          writer.String("volts");
          writer.Double(mx28->presentVoltage, "%.1f");
        }
        writer.EndObject();
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}
