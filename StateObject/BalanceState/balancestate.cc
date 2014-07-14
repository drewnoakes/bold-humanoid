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
      writer.String("hipRollR").Double(d_offsets.hipRollR);
      writer.String("hipRollL").Double(d_offsets.hipRollL);
      writer.String("kneeR").Double(d_offsets.kneeR);
      writer.String("kneeL").Double(d_offsets.kneeL);
      writer.String("anklePitchR").Double(d_offsets.anklePitchR);
      writer.String("anklePitchL").Double(d_offsets.anklePitchL);
      writer.String("ankleRollR").Double(d_offsets.ankleRollR);
      writer.String("ankleRollL").Double(d_offsets.ankleRollL);
    }
    writer.EndObject();
  }
  writer.EndObject();
}
