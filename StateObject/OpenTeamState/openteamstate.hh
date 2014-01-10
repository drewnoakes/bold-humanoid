#pragma once

#include "../stateobject.hh"
#include "../../mitecom/mitecom-data.h"

namespace bold
{
  class OpenTeamState : public StateObject
  {
  public:
    OpenTeamState(MixedTeamMates teamMates)
    : d_teamMates(teamMates)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    MixedTeamMates d_teamMates;
  };
}
