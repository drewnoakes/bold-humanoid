#pragma once

#include <memory>
#include <vector>

#include "../stateobject.hh"

namespace bold
{
  class StaticCM730State;
  class StaticMX28State;

  class StaticHardwareState : public StateObject
  {
  public:
    StaticHardwareState(std::shared_ptr<StaticCM730State const> cm730State,
                        std::vector<std::shared_ptr<StaticMX28State const>> mx28States)
    : d_cm730State(cm730State),
      d_mx28States(mx28States)
    {}

    std::shared_ptr<StaticCM730State const> getCM730State() const
    {
      return d_cm730State;
    }

    std::shared_ptr<StaticMX28State const> getMX28State(unsigned jointId) const
    {
      assert(jointId > 0 && jointId <= NUMBER_OF_JOINTS);
      assert(d_mx28States.size() > jointId);

      return d_mx28States[jointId];
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    const int NUMBER_OF_JOINTS = 20;

    std::shared_ptr<StaticCM730State const> d_cm730State;
    std::vector<std::shared_ptr<StaticMX28State const>> d_mx28States;
  };
}
