#pragma once

#include "../../Config/config.hh"
#include "../../Math/math.hh"
#include "../option.hh"

namespace bold
{
  class WalkModule;
  class BehaviourControl;

  class ApproachBall : public Option
  {
  public:
    ApproachBall(std::string const& id, std::shared_ptr<WalkModule> walkModule, std::shared_ptr<BehaviourControl> behaviourControl)
    : Option(id, "ApproachBall"),
      d_walkModule(walkModule),
      d_behaviourControl(behaviourControl)
    {}

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;
    
    double hasTerminated() override;

    void setStopDistance(double stopDistance)
    {
      d_useCustomStopDistance = true;
      d_stopDistance = stopDistance;
    }

  private:
    std::shared_ptr<WalkModule> d_walkModule;
    std::shared_ptr<BehaviourControl> d_behaviourControl;
    bool d_useCustomStopDistance;
    double d_stopDistance;
  };
}
