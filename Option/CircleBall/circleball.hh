#pragma once

#include "../option.hh"
#include "../../Clock/clock.hh"

#include <Eigen/Core>

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

    virtual double hasTerminated() override;

    void setTurnParams(double turnAngleRads, Eigen::Vector2d targetBallPos);

  private:
    std::shared_ptr<WalkModule> d_walkModule;
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<LookAtFeet> d_lookAtFeet;
    std::shared_ptr<LookAtBall> d_lookAtBall;
    Eigen::Vector2d d_targetBallPos;
    double d_targetYaw;
  };
}
