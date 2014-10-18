#pragma once

#include "../option.hh"

namespace bold
{
  class Agent;
  class HeadModule;
  class LookAtFeet;
  class LookAround;

  /** Controls the head while standing at the ball to build the stationary map.
   */
  class AtBall : public Option
  {
  public:
    AtBall(std::string const& id, Agent* agent);

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void reset() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<LookAtFeet> d_lookAtFeetOption;
    std::shared_ptr<LookAround> d_lookAroundOption;
  };
}
