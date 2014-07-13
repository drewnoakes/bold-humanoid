#pragma once

#include "../../Agent/agent.hh"
#include "../LookAtBall/lookatball.hh"
#include "../option.hh"

namespace bold
{
  class Agent;
  class HeadModule;
  class LookAtFeet;
  class LookAround;

  /** Controls the head while standing still, in order to find the ball.
  */
  class LocateBall : public Option
  {
  public:
    LocateBall(std::string const& id, Agent* agent, std::function<double(uint)> speedCallback = nullptr, uint maxCount = 10, uint thresholdCount = 5);

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<LookAround> d_lookAroundOption;
    std::shared_ptr<LookAtBall> d_lookAtBallOption;
    uint d_visibleCount;
    uint d_stepCount;
    uint d_maxCount;
    uint d_thresholdCount;
  };
}
