#pragma once

#include "../stateobject.hh"
#include "../../Balance/balance.hh"

namespace bold
{
  class BalanceState : public StateObject
  {
  public:
    BalanceState(BalanceOffset const& offsets);

    BalanceOffset const& offsets() const { return d_offsets; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    BalanceOffset d_offsets;
  };
}
