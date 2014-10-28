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
