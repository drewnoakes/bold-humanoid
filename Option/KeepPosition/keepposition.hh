#pragma once

#include "../option.hh"
#include "../../StateObject/TeamState/teamstate.hh"

namespace bold
{
  class Agent;
  class ApproachBall;
  template<typename> class Setting;
  class WalkModule;

  class KeepPosition : public Option
  {
  public:
    KeepPosition(std::string id, PlayerRole role, Agent* agent);

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    std::shared_ptr<WalkModule> d_walkModule;
    PlayerRole d_role;
    std::shared_ptr<ApproachBall> d_approachBall;
    Setting<double>* d_supporterSpacing;
  };
}
