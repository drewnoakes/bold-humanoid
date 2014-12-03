#pragma once

#include "../stateobject.hh"

namespace bold
{
  class MessageCountState : public StateObject
  {
  public:
    MessageCountState(unsigned gameControllerMessageCount,
                      unsigned ignoredMessageCount,
                      unsigned sentTeamMessageCount,
                      unsigned receivedTeamMessageCount,
                      unsigned sentDrawbridgeMessageCount)
    : d_gameControllerMessageCount(gameControllerMessageCount),
      d_ignoredMessageCount(ignoredMessageCount),
      d_sentTeamMessageCount(sentTeamMessageCount),
      d_receivedTeamMessageCount(receivedTeamMessageCount),
      d_sentDrawbridgeMessageCount(sentDrawbridgeMessageCount)
    {}


    unsigned int getGameControllerMessageCount() const { return d_gameControllerMessageCount; }
    unsigned int getIgnoredMessageCount() const { return d_ignoredMessageCount; }
    unsigned int getSentTeamMessageCount() const { return d_sentTeamMessageCount; }
    unsigned int getReceivedTeamMessageCount() const { return d_receivedTeamMessageCount; }
    unsigned int getSentDrawbridgeMessageCount() const { return d_sentDrawbridgeMessageCount; }

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
  };

  template<typename TBuffer>
  inline void MessageCountState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
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
    }
    writer.EndObject();
  }
}
