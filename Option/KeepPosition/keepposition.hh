#pragma once

#include "../option.hh"
#include "../../StateObject/TeamState/teamstate.hh"

namespace bold
{
  class Ambulator;
  class ApproachBall;
  template<typename> class Setting;

  class KeepPosition : public Option
  {
  public:
    KeepPosition(std::string id, PlayerRole role, std::shared_ptr<Ambulator> ambulator);

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
    PlayerRole d_role;
    std::shared_ptr<ApproachBall> d_approachBall;
    Setting<double>* d_supporterSpacing;
  };
}
