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
      writer.String("hipRollR").Int(d_offsets.hipRollR);
      writer.String("hipRollL").Int(d_offsets.hipRollL);
      writer.String("kneeR").Int(d_offsets.kneeR);
      writer.String("kneeL").Int(d_offsets.kneeL);
      writer.String("anklePitchR").Int(d_offsets.anklePitchR);
      writer.String("anklePitchL").Int(d_offsets.anklePitchL);
      writer.String("ankleRollR").Int(d_offsets.ankleRollR);
      writer.String("ankleRollL").Int(d_offsets.ankleRollL);
    }
    writer.EndObject();
  }
  writer.EndObject();
}
