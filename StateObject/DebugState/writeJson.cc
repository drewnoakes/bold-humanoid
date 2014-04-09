#include "debugstate.hh"

using namespace std;
using namespace bold;
using namespace rapidjson;

void DebugState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject()
    .String("gameControllerMessages")
      .Int(d_gameControllerMessageCount)
    .String("ignoredMessages")
      .Int(d_ignoredMessageCount)
    .String("sentTeamMessages")
      .Int(d_sentTeamMessageCount)
    .String("receivedTeamMessages")
      .Int(d_receivedTeamMessageCount)
    .String("sentDrawbridgeMessages")
      .Int(d_sentDrawbridgeMessageCount)
    .String("eyeColour")
      .StartArray()
        .Int(d_eyeColour.r)
        .Int(d_eyeColour.g)
        .Int(d_eyeColour.b)
      .EndArray()
    .String("foreheadColour")
      .StartArray()
        .Int(d_foreheadColour.r)
        .Int(d_foreheadColour.g)
        .Int(d_foreheadColour.b)
      .EndArray()
    .String("led")
      .StartArray()
        .Bool(d_redLed)
        .Bool(d_greenLed)
        .Bool(d_blueLed)
      .EndArray()
  .EndObject();
}
