#pragma once

#include "../option.hh"

namespace bold
{
  class Agent;
  class WalkModule;
  class HeadModule;
  class LookAtFeet;
  class LookAtBall;

  class CircleBall : public Option
  {
  public:
    CircleBall(std::string const& id, Agent* agent);

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void setIsLeftTurn(bool leftTurn);

    virtual void reset() override;

  private:
    std::shared_ptr<WalkModule> d_walkModule;
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<LookAtFeet> d_lookAtFeet;
    std::shared_ptr<LookAtBall> d_lookAtBall;
    bool d_isLeftTurn;
  };
}
