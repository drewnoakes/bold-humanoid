#pragma once

#include <memory>
#include <vector>

#include "../stateobject.hh"
#include "../../robotis/Framework/include/JointData.h"

namespace bold
{
  class CM730Snapshot;
  class MX28Snapshot;

  class HardwareState : public StateObject
  {
  public:
    HardwareState(std::shared_ptr<CM730Snapshot const> cm730State, std::vector<std::shared_ptr<MX28Snapshot const>> mx28States)
    : d_cm730State(cm730State),
      d_mx28States(mx28States)
    {}

    std::shared_ptr<CM730Snapshot const> getCM730State() const
    {
      return d_cm730State;
    }

    std::shared_ptr<MX28Snapshot const> getMX28State(unsigned jointId) const
    {
      assert(jointId > 0 && jointId < Robot::JointData::NUMBER_OF_JOINTS);
      assert(d_mx28States.size() > jointId);

      return d_mx28States[jointId];
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::shared_ptr<CM730Snapshot const> d_cm730State;
    std::vector<std::shared_ptr<MX28Snapshot const>> d_mx28States;
  };
}
