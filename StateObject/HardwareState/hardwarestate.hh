#pragma once

#include <memory>
#include <vector>

#include "../stateobject.hh"

namespace bold
{
  class CM730Snapshot;
  class MX28Snapshot;

  class HardwareState : public StateObject
  {
  public:
    HardwareState(std::shared_ptr<CM730Snapshot const> cm730State,
                  std::vector<std::shared_ptr<MX28Snapshot const>> mx28States,
                  unsigned long rxBytes,
                  unsigned long txBytes)
    : d_cm730State(cm730State),
      d_mx28States(mx28States),
      d_rxBytes(rxBytes),
      d_txBytes(txBytes)
    {}

    std::shared_ptr<CM730Snapshot const> getCM730State() const
    {
      return d_cm730State;
    }

    std::shared_ptr<MX28Snapshot const> getMX28State(unsigned jointId) const
    {
      assert(jointId > 0 && jointId <= NUMBER_OF_JOINTS);
      assert(d_mx28States.size() > jointId);

      return d_mx28States[jointId];
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    unsigned long GetReceivedBytes() const { return d_rxBytes; }
    unsigned long GetTransmittedBytes() const { return d_txBytes; }

  private:
    const int NUMBER_OF_JOINTS = 20;

    std::shared_ptr<CM730Snapshot const> d_cm730State;
    std::vector<std::shared_ptr<MX28Snapshot const>> d_mx28States;
    unsigned long d_rxBytes;
    unsigned long d_txBytes;
  };
}
