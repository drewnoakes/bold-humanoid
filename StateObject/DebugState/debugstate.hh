#pragma once

#include "../stateobject.hh"
#include "../../DebugControl/debugcontrol.hh"
#include "../../Colour/colour.hh"

#include <memory>

namespace bold
{
  class DebugState : public StateObject
  {
  public:
    DebugState(unsigned gameControllerMessageCount, unsigned ignoredMessageCount,
               unsigned sentTeamMessageCount, unsigned receivedTeamMessageCount,
               unsigned sentDrawbridgeMessageCount,
               std::shared_ptr<DebugControl> debugControl)
    : d_gameControllerMessageCount(gameControllerMessageCount),
      d_ignoredMessageCount(ignoredMessageCount),
      d_sentTeamMessageCount(sentTeamMessageCount),
      d_receivedTeamMessageCount(receivedTeamMessageCount),
      d_sentDrawbridgeMessageCount(sentDrawbridgeMessageCount),
      d_eyeColour(debugControl->getEyeColour()),
      d_foreheadColour(debugControl->getForeheadColour()),
      d_redLed(debugControl->isRedPanelLedLit()),
      d_greenLed(debugControl->isGreenPanelLedLit()),
      d_blueLed(debugControl->isBluePanelLedLit())
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;
    unsigned d_sentTeamMessageCount;
    unsigned d_receivedTeamMessageCount;
    unsigned d_sentDrawbridgeMessageCount;
    Colour::bgr d_eyeColour;
    Colour::bgr d_foreheadColour;
    bool d_redLed;
    bool d_greenLed;
    bool d_blueLed;
  };

  template<typename TBuffer>
  inline void DebugState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("gameControllerMessages");
      writer.Int(d_gameControllerMessageCount);
      writer.String("ignoredMessages");
      writer.Int(d_ignoredMessageCount);
      writer.String("sentTeamMessages");
      writer.Int(d_sentTeamMessageCount);
      writer.String("receivedTeamMessages");
      writer.Int(d_receivedTeamMessageCount);
      writer.String("sentDrawbridgeMessages");
      writer.Int(d_sentDrawbridgeMessageCount);
      writer.String("eyeColour");
      writer.StartArray();
      {
        writer.Int(d_eyeColour.r);
        writer.Int(d_eyeColour.g);
        writer.Int(d_eyeColour.b);
      }
      writer.EndArray();
      writer.String("foreheadColour");
      writer.StartArray();
      {
        writer.Int(d_foreheadColour.r);
        writer.Int(d_foreheadColour.g);
        writer.Int(d_foreheadColour.b);
      }
      writer.EndArray();
      writer.String("led");
      writer.StartArray();
      {
        writer.Bool(d_redLed);
        writer.Bool(d_greenLed);
        writer.Bool(d_blueLed);
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}
