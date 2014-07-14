#include "balancestate.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

BalanceState::BalanceState(BalanceOffset const& offsets)
: d_offsets(offsets)
{}

void BalanceState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("offsets");
    writer.StartObject();
    {
      writer.String("hipRoll").Int(d_offsets.hipRoll);
      writer.String("knee").Int(d_offsets.knee);
      writer.String("anklePitch").Int(d_offsets.anklePitch);
      writer.String("ankleRoll").Int(d_offsets.ankleRoll);
    }
    writer.EndObject();
  }
  writer.EndObject();
}
