#pragma once

#include "../option.hh"
#include "../../Clock/clock.hh"

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

    virtual void reset() override;

    virtual double hasTerminated() override;

    void setTurnAngle(double angleRads);

  private:
    std::shared_ptr<WalkModule> d_walkModule;
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<LookAtFeet> d_lookAtFeet;
    std::shared_ptr<LookAtBall> d_lookAtBall;
    double d_turnAngleRads;
    double d_durationSeconds;
    Clock::Timestamp d_startTime;
  };
}
