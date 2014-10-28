#include "debugstate.hh"

using namespace std;
using namespace bold;
using namespace rapidjson;

void DebugState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
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
  writer.Int(d_eyeColour.r);
  writer.Int(d_eyeColour.g);
  writer.Int(d_eyeColour.b);
  writer.EndArray();
  writer.String("foreheadColour");
  writer.StartArray();
  writer.Int(d_foreheadColour.r);
  writer.Int(d_foreheadColour.g);
  writer.Int(d_foreheadColour.b);
  writer.EndArray();
  writer.String("led");
  writer.StartArray();
  writer.Bool(d_redLed);
  writer.Bool(d_greenLed);
  writer.Bool(d_blueLed);
  writer.EndArray();
  writer.EndObject();
}
