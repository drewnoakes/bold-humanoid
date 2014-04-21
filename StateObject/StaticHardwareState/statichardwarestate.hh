#pragma once

#include <memory>
#include <vector>

#include "../stateobject.hh"
#include "../../JointId/jointid.hh"
#include "../../util/assert.hh"

namespace bold
{
  typedef unsigned char uchar;

  class StaticCM730State;
  class StaticMX28State;

  class StaticHardwareState : public StateObject
  {
  public:
    StaticHardwareState(std::shared_ptr<StaticCM730State const> cm730State,
                        std::vector<std::shared_ptr<StaticMX28State const>> mx28States)
    : d_cm730State(cm730State),
      d_mx28States(mx28States)
    {
      ASSERT(d_mx28States.size() == (uchar)JointId::MAX);
    }

    std::shared_ptr<StaticCM730State const> getCM730State() const
    {
      return d_cm730State;
    }

    std::shared_ptr<StaticMX28State const> getMX28State(uchar jointId) const
    {
      ASSERT(jointId >= (uchar)JointId::MIN && jointId <= (uchar)JointId::MAX);

      return d_mx28States[jointId - 1];
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::shared_ptr<StaticCM730State const> d_cm730State;
    std::vector<std::shared_ptr<StaticMX28State const>> d_mx28States;
  };
}
