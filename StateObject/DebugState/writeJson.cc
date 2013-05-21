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
    .String("eyeColour")
      .StartArray()
        .Int(d_eyeColour.r)
        .Int(d_eyeColour.g)
        .Int(d_eyeColour.b)
      .EndArray()
    .String("headColour")
      .StartArray()
        .Int(d_headColour.r)
        .Int(d_headColour.g)
        .Int(d_headColour.b)
      .EndArray()
    .String("led")
      .StartArray()
        .Bool(d_redLed)
        .Bool(d_greenLed)
        .Bool(d_blueLed)
      .EndArray()
  .EndObject();
}