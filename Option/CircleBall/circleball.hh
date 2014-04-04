#pragma once

#include "../option.hh"

namespace bold
{
  class Ambulator;
  class HeadModule;
  class LookAtFeet;
  class LookAtBall;

  class CircleBall : public Option
  {
  public:
    CircleBall(std::string const& id, std::shared_ptr<Ambulator> ambulator, std::shared_ptr<HeadModule> headModule, std::shared_ptr<LookAtFeet> lookAtFeet, std::shared_ptr<LookAtBall> lookAtBall)
    : Option(id, "CircleBall"),
      d_ambulator(ambulator),
      d_headModule(headModule),
      d_lookAtFeet(lookAtFeet),
      d_lookAtBall(lookAtBall),
      d_isLeftTurn(true)
    {}

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void setIsLeftTurn(bool leftTurn);

    virtual void reset() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<LookAtFeet> d_lookAtFeet;
    std::shared_ptr<LookAtBall> d_lookAtBall;
    bool d_isLeftTurn;
  };
}
