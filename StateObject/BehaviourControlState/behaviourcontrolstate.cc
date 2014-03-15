#include "behaviourcontrolstate.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../BehaviourControl/behaviourcontrol.hh"

using namespace bold;
using namespace std;
using namespace rapidjson;

typedef unsigned int uint;

void BehaviourControlState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("role").Uint(static_cast<uint>(d_playerRole));
    writer.String("activity").Uint(static_cast<uint>(d_playerActivity));
    writer.String("status").Uint(static_cast<uint>(d_playerStatus));
  }
  writer.EndObject();
}
