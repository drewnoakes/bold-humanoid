#pragma once

#include <memory>
#include <vector>

#include "../stateobject.hh"
#include "../../JointId/jointid.hh"
#include "../../util/assert.hh"

namespace bold
{
  typedef unsigned char uchar;

  class StaticCM730State;
  class StaticMX28State;

  class StaticHardwareState : public StateObject
  {
  public:
    StaticHardwareState(std::shared_ptr<StaticCM730State const> cm730State,
                        std::vector<std::shared_ptr<StaticMX28State const>> mx28States)
    : d_cm730State(cm730State),
      d_mx28States(mx28States)
    {
      ASSERT(d_mx28States.size() == (uchar)JointId::MAX);
    }

    std::shared_ptr<StaticCM730State const> getCM730State() const
    {
      return d_cm730State;
    }

    std::shared_ptr<StaticMX28State const> getMX28State(uchar jointId) const
    {
      ASSERT(jointId >= (uchar)JointId::MIN && jointId <= (uchar)JointId::MAX);

      return d_mx28States[jointId - 1];
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJson(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJson(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJson(writer); }

    template<typename TWriter>
    void writeJson(TWriter &writer) const;

  private:
    std::shared_ptr<StaticCM730State const> d_cm730State;
    std::vector<std::shared_ptr<StaticMX28State const>> d_mx28States;
  };

  template<typename TWriter>
  inline void StaticHardwareState::writeJson(TWriter &writer) const
  {
    auto writeAlarm = [&writer](std::string name, MX28Alarm const& alarm)
    {
      writer.String(name.c_str());
      writer.StartArray();
      {
        for (auto const& setName : alarm.getSetNames())
          writer.String(setName.c_str());
      }
      writer.EndArray();
    };

    writer.StartObject();
    {
      writer.String("id");
      writer.Int(d_cm730State->dynamixelId);
      writer.String("baud");
      writer.Int(d_cm730State->baudBPS);
      writer.String("firmwareVersion");
      writer.Int(d_cm730State->firmwareVersion);
      writer.String("modelNumber");
      writer.Int(d_cm730State->modelNumber);
      writer.String("returnDelayTimeMicroSeconds");
      writer.Int(d_cm730State->returnDelayTimeMicroSeconds);
      writer.String("statusRetLevel");
      writer.Int(d_cm730State->statusRetLevel);

      writer.String("joints");
      writer.StartArray();
      for (std::shared_ptr<StaticMX28State const> mx28 : d_mx28States)
      {
        writer.StartObject();
        {
          writer.String("id");
          writer.Int(mx28->id);
          writer.String("modelNumber");
          writer.Int(mx28->modelNumber);
          writer.String("firmwareVersion");
          writer.Int(mx28->firmwareVersion);
          writer.String("baud");
          writer.Int(mx28->baudBPS);
          writer.String("returnDelayTimeMicroSeconds");
          writer.Int(mx28->returnDelayTimeMicroSeconds);
          writer.String("angleLimitCW");
          writer.Double(mx28->angleLimitCW, "%.4f");
          writer.String("angleLimitCCW");
          writer.Double(mx28->angleLimitCCW, "%.4f");
          writer.String("tempLimitHighCelsius");
          writer.Int(mx28->tempLimitHighCelsius);
          writer.String("voltageLimitLow");
          writer.Double(mx28->voltageLimitLow);
          writer.String("voltageLimitHigh");
          writer.Double(mx28->voltageLimitHigh);
          writer.String("maxTorque");
          writer.Int(mx28->maxTorque);
          writer.String("statusRetLevel");
          writer.Int(mx28->statusRetLevel);
          writeAlarm("alarmLed", mx28->alarmLed);
          writeAlarm("alarmShutdown", mx28->alarmShutdown);
          writer.String("torqueEnable");
          writer.Bool(mx28->torqueEnable);
          writer.String("isEepromLocked");
          writer.Bool(mx28->isEepromLocked);
        }
        writer.EndObject();
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}
